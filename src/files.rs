use std::io::{Error, ErrorKind};
use std::env;
use std::path::Path;
use walkdir::WalkDir;
use crate::md;

pub struct PrimaryPaths {
    pub base_dir: String,
    pub source: String,
    pub destination: String,
}

impl PrimaryPaths {
    pub fn new(src: &str, dest: &str) -> Self {
        PrimaryPaths {
            base_dir: env::current_dir().unwrap().to_str().unwrap().to_string(),
            source: src.to_string(),
            destination: dest.to_string(),
        }
    }

    pub fn paths_check(&mut self) -> Result<(), Error> {
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

    pub fn traverse(&mut self) -> Result<(), Error> {
        for entry in WalkDir::new(&self.source).min_depth(1).follow_links(true) {
            let entry = entry?.clone();
            let metadata = entry.metadata()?;
            if !metadata.is_dir() {
                println!("{}", entry.path().display());
                if let Some(ext) = entry.path().extension() {
                    match ext.to_str() {
                        Some("md") => {
                            let path = entry.path().to_str().unwrap();
                            match md::convert_to_html(path) {
                                Err(why) => {
                                    eprintln!("Cannot convert markdown to html: {} | File: {}",
                                        why, path);
                                }
                                Ok(html_output) => {
                                    println!("{}", &html_output);
                                }
                            }
                        },
                        Some("html") | Some("htm") => (),
                        _ => (),
                    }
                } else {
                    eprintln!("Cannot read entry extension for: {}",
                        entry.path().display());
                }
            }
        }

        Ok(())
    }
}

