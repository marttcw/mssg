use std::{fs, env, error::Error, io::prelude::*, path::Path};

// Initialize a new site project
pub fn init(name: &str) {
    if name != "" {
        let _result = fs::create_dir(name);
        match _result {
            Ok(_) => {
                let proj_dir = Path::new(name);
                assert!(env::set_current_dir(&proj_dir).is_ok());
                println!("Directory \"{}\" created", proj_dir.display());
            }
            Err(e) => {
                eprintln!("Error occurred: {}", e);
                return;
            }
        }
    }

    let mut _result;
    _result = fs::create_dir("src");
    _result = fs::create_dir("build");
    match _result {
        Ok(_) => println!("Project successfully initialized"),
        Err(e) => {
            eprintln!("Error occurred: {}", e);
            return;
        }
    }
}
