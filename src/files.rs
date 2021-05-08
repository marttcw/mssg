use crate::{blog, md};
use std::{
    env,
    fs::{create_dir_all, read_to_string, File},
    io::{prelude::*, BufReader, Error, ErrorKind, Result},
    path::{Path, PathBuf},
};
use walkdir::WalkDir;

pub struct PrimaryPaths {
    pub base_dir: String,
    pub source: String,
    pub destination: String,
    pub files_priority: Vec<SrcFileInfo>,
    pub files: Vec<SrcFileInfo>,
}

#[derive(PartialEq)]
pub enum FileType {
    HTML,
    HTMLBase,
    MarkDown,
    Other(String),
}

impl FileType {
    pub fn from_ext(ext: &str, file_name: &str) -> FileType {
        match ext {
            "html" | "htm" => {
                if file_name == "_base.html" {
                    FileType::HTMLBase
                } else {
                    FileType::HTML
                }
            }
            "md" => FileType::MarkDown,
            _ => FileType::Other(String::from(ext)),
        }
    }

    /*
    pub fn to_str(&self) -> &str {
        match self {
            FileType::HTML => "html",
            FileType::MarkDown => "md",
            FileType::Other(ext) => ext,
        }
    }
    */
}

pub struct SrcFileInfo {
    pub path: String,
    pub file_name: String,
    pub file_type: FileType,
    pub dst_path: String,
    pub dst_file_name: String,
}

impl PrimaryPaths {
    pub fn new(src: &str, dest: &str) -> Self {
        PrimaryPaths {
            base_dir: env::current_dir().unwrap().to_str().unwrap().to_string(),
            source: src.to_string(),
            destination: dest.to_string(),
            files: vec![],
            files_priority: vec![],
        }
    }

    pub fn paths_check(&mut self) -> Result<()> {
        let ok_source = Path::new(&self.source).exists();
        let ok_destination = Path::new(&self.destination).exists();

        if ok_destination && ok_source {
            Ok(())
        } else {
            eprintln!("Source: {} found status: {}", self.source, ok_source);
            eprintln!(
                "Destination: {} found status: {}",
                self.destination, ok_destination
            );
            Err(Error::new(ErrorKind::NotFound, "Paths not found"))
        }
    }

    fn omit_dst(&self, path: &str, file_type: &FileType) -> PathBuf {
        let mut new_dst_str = self.destination.clone();
        new_dst_str += &path[self.source.len()..];
        let mut new_dst = PathBuf::new();
        new_dst.push(new_dst_str);
        match *file_type {
            FileType::MarkDown => {
                new_dst.set_extension("html");
            }
            _ => (),
        }
        new_dst
    }

    pub fn traverse(&mut self) -> Result<()> {
        for entry in WalkDir::new(&self.source).min_depth(1).follow_links(true) {
            let entry = entry?.clone();
            let metadata = entry.metadata()?;
            if !metadata.is_dir() {
                if let Some(ext) = entry.path().extension() {
                    let file_type = FileType::from_ext(
                        ext.to_str().unwrap(),
                        entry.file_name().to_str().unwrap(),
                    );
                    let dst_path = self.omit_dst(entry.path().to_str().unwrap(), &file_type);
                    let to_file = if file_type == FileType::HTMLBase {
                        &mut self.files_priority
                    } else {
                        &mut self.files
                    };
                    to_file.push(SrcFileInfo {
                        path: String::from(entry.path().to_str().unwrap()),
                        file_name: String::from(entry.file_name().to_str().unwrap()),
                        file_type: file_type,
                        dst_path: String::from(dst_path.to_str().unwrap()),
                        dst_file_name: String::from(
                            dst_path.file_name().unwrap().to_str().unwrap(),
                        ),
                    });
                } else {
                    eprintln!(
                        "Cannot read entry extension for: {}",
                        entry.path().display()
                    );
                }
            }
        }

        Ok(())
    }

    pub fn build(&mut self) -> Result<()> {
        let mut file_base_header = String::from("");
        let mut file_base_footer = String::from("");
        for file in &self.files_priority {
            println!("{}", file.path);
            match file.file_type {
                FileType::HTMLBase => {
                    let file = File::open(&file.path)?;
                    let reader = BufReader::new(file);
                    let mut read_header = true;

                    for line in reader.lines() {
                        match line {
                            Ok(line) => {
                                if line.starts_with("<!--@CONTENT@-->") {
                                    read_header = false;
                                } else if read_header {
                                    file_base_header += &line;
                                    file_base_header.push('\n');
                                } else {
                                    file_base_footer += &line;
                                    file_base_footer.push('\n');
                                }
                            }
                            Err(_) => (),
                        }
                    }
                }
                _ => (),
            }
        }

        for file in &self.files {
            let mut file_content = String::from("");
            println!("{} -> {}", file.path, file.dst_path);
            match file.file_type {
                FileType::HTML => {
                    file_content += &file_base_header;
                    file_content += &read_to_string(&file.path)?;
                    file_content += &file_base_footer;
                }
                FileType::MarkDown => {
                    let mut post_proc_filestr = String::new();
                    let fo = File::open(&file.path)?;
                    let reader = BufReader::new(fo);
                    for line in reader.lines() {
                        match line {
                            Ok(line) => {
                                if line.starts_with("<!--@LIST@-->") {
                                    let entries = blog::get_list("src/blog")?;
                                    for entry in entries {
                                        post_proc_filestr += &entry.to_md_li("src");
                                    }
                                } else {
                                    post_proc_filestr += &line;
                                }
                                post_proc_filestr.push('\n');
                            }
                            Err(_) => (),
                        }
                    }

                    match md::convert_str_to_html(&post_proc_filestr) {
                        Ok(html_output) => {
                            file_content += &file_base_header;
                            file_content += &html_output;
                            file_content += &file_base_footer;
                        }
                        Err(why) => {
                            eprintln!(
                                "Cannot convert markdown to html: {} | File: {}",
                                why, file.path
                            );
                        }
                    }
                }
                FileType::HTMLBase => (),
                FileType::Other(_) => (),
            }

            if file_content != "" {
                {
                    // Create destination directory path
                    let mut dst_dir_paths = String::from(&file.dst_path);
                    for _ in 0..file.dst_file_name.len() {
                        dst_dir_paths.pop();
                    }
                    //println!("NEWPATH: {}", &dst_dir_paths);
                    create_dir_all(&dst_dir_paths)?;
                }

                // Create file and write to it
                match File::create(&file.dst_path) {
                    Ok(mut file) => {
                        file.write_all(&file_content.into_bytes())?;
                    }
                    Err(why) => {
                        println!("ERROR: {}: {}", &file.dst_path, why);
                    }
                }
            }
        }

        Ok(())
    }
}
