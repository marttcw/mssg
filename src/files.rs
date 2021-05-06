use std::{io::{Result, Error, ErrorKind, BufReader, prelude::*}, env, fs::{File, read_to_string}, path::Path};
use walkdir::WalkDir;
use crate::md;

pub struct PrimaryPaths {
    pub base_dir: String,
    pub source: String,
    pub destination: String,
    pub files: Vec<SrcFileInfo>,
}

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
            },
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
}

impl PrimaryPaths {
    pub fn new(src: &str, dest: &str) -> Self {
        PrimaryPaths {
            base_dir: env::current_dir().unwrap().to_str().unwrap().to_string(),
            source: src.to_string(),
            destination: dest.to_string(),
            files: vec![],
        }
    }

    pub fn paths_check(&mut self) -> Result<()> {
        let ok_source = Path::new(&self.source).exists();
        let ok_destination = Path::new(&self.destination).exists();

        if ok_destination && ok_source {
            Ok(())
        } else {
            eprintln!("Source: {} found status: {}", self.source, ok_source);
            eprintln!("Destination: {} found status: {}", self.destination, ok_destination);
            Err(Error::new(ErrorKind::NotFound, "Paths not found"))
        }
    }

    fn omit_dst(&self, path: &str) -> String {
        let mut new_dst = self.destination.clone();
        new_dst += &path[self.source.len()..];
        new_dst
    }

    pub fn traverse(&mut self) -> Result<()> {
        for entry in WalkDir::new(&self.source).min_depth(1).follow_links(true) {
            let entry = entry?.clone();
            let metadata = entry.metadata()?;
            if !metadata.is_dir() {
                if let Some(ext) = entry.path().extension() {
                    self.files.push(SrcFileInfo{
                        path: String::from(entry.path().to_str().unwrap()),
                        file_name: String::from(entry.file_name().to_str().unwrap()),
                        file_type: FileType::from_ext(ext.to_str().unwrap(), entry.file_name().to_str().unwrap()), 
                        dst_path: self.omit_dst(entry.path().to_str().unwrap()),
                    });
                } else {
                    eprintln!("Cannot read entry extension for: {}",
                        entry.path().display());
                }
            }
        }

        Ok(())
    }

    pub fn build(&mut self) -> Result<()> {
        let mut file_base_header = String::from("");
        let mut file_base_footer = String::from("");
        for file in &self.files {
            let mut file_content = String::from("");
            print!("{} ({} -> {}) ", file.file_name, file.path, file.dst_path);
            match file.file_type {
                FileType::HTML => {
                    file_content = read_to_string(&file.path)?;
                    println!("HTML");
                },
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
                            },
                            Err(_) => (),
                        }
                    }

                    //println!("{}", file_base_header);
                    //println!("{}", file_base_footer);
                    println!("HTMLBase");
                },
                FileType::MarkDown => {
                    println!("MarkDown");
                    match md::convert_to_html(&file.path) {
                        Ok(html_output) => {
                            file_content = html_output;
                        }
                        Err(why) => {
                            eprintln!("Cannot convert markdown to html: {} | File: {}",
                                why, file.path);
                        }
                    }
                },
                FileType::Other(_) => (),
            }
            //println!("{}", file_content);
        }

        Ok(())
    }
}

