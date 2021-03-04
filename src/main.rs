mod cli;

use std::io::Error;
use std::{env, fs};

fn main() -> Result<(), Error> {
    match cli::parse().subcmd {
        cli::SubCommand::Build(_) => {
            let base_dir = env::current_dir()?;
            if let Some(s) = base_dir.to_str() {
                println!("Base Dir: {}", s);
            }

            // Find source and destination directory
            for entry in fs::read_dir(base_dir)? {
                if let Ok(entry) = entry {
                    if let Ok(metadata) = entry.metadata() {
                        if metadata.is_dir() {
                            match entry.file_name().to_str() {
                                Some("src") => {
                                    if let Some(s) = entry.path().to_str() {
                                        println!("Source directory: {}", s);
                                    }
                                },
                                Some("dst") => {
                                    if let Some(s) = entry.path().to_str() {
                                        println!("Destination directory: {}", s);
                                    }
                                },
                                _ => (),
                            }
                        }
                    }
                }
            }
        },
    }
    Ok(())
}

