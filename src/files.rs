use std::io::Error;
use std::{env, fs};
//use walkdir::WalkDir;

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

    pub fn paths_check(&mut self) -> Result<bool, Error> {
        let mut ok_source = false;
        let mut ok_destination = false;

        // Find source and destination directory
        for entry in fs::read_dir(&self.base_dir)? {
            if let Ok(entry) = entry {
                if let Ok(metadata) = entry.metadata() {
                    if metadata.is_dir() {
                        if self.source == entry.file_name().to_str().unwrap() {
                            ok_source = true;
                        } else if self.destination == entry.file_name().to_str().unwrap() {
                            ok_destination = true;
                        }
                    }
                }
            }
        }

        Ok(ok_destination && ok_source)
    }
}

