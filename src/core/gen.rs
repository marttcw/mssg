use std::{fs, env, io::BufReader, io::prelude::*, path::Path, error::Error};
use walkdir::{DirEntry, WalkDir};
use pulldown_cmark::{Parser, Options, html};
use toml::Value;
use serde::Deserialize;

#[derive(Deserialize)]
struct Config {
    title: String,
    description: String,
    language: String,
    global: Global_Config,
}

#[derive(Deserialize)]
struct Global_Config {
    order: Vec<String>
}

struct path_file {
    rel_path: String,
    filename: String
}

struct build_map_to {
    build_file: String,
    src_files_list: Vec<String>
}

// Generate the site
pub fn gen() -> Result<(), Box<dyn Error>> {
    let mut list: Vec<path_file> = Vec::new();
    let mut equiv: Vec<build_map_to> = Vec::new();

    get_list_files(&mut list, "src")?;

    for f in &list {
        println!("{} : {}", f.rel_path, f.filename);
    }

    markdown("src/index.md", "src/config.toml", "build/index.html")?;

    Ok(())
}

// Walk directory traversal
fn is_not_hidden(entry: &DirEntry) -> bool {
    entry
         .file_name()
         .to_str()
         .map(|s| entry.depth() == 0 || !s.starts_with("."))
         .unwrap_or(false)
}

fn get_list_files(list: &mut Vec<path_file>, base_dir_path: &str) -> Result<(), Box<dyn Error>> {
    WalkDir::new(base_dir_path)
        .into_iter()
        .filter_entry(|e| is_not_hidden(e))
        .filter_map(|v| v.ok())
        .for_each(|x| 
            list.push(
                path_file{
                    rel_path: x.path().display().to_string(),
                    filename: x.path().file_name().unwrap().to_os_string().into_string().unwrap()
                }));

    Ok(())
}

// Markdown generator
fn markdown(in_filename: &str, in_config: &str, out_filename: &str) -> std::io::Result<()> {
    let mut html_output = String::new();

    // Open file and read it to string
    let contents = fs::read_to_string(in_filename).expect("Unable to read file");
    let config_contents = fs::read_to_string(in_config).expect("Unable to read configuration file");

    // Parse the markdown contents to HTML
    let mut options = Options::empty();
    options.insert(Options::ENABLE_STRIKETHROUGH);
    let parser = Parser::new_ext(&*contents, options);
    html::push_html(&mut html_output, parser);

    // Parse the configuration file from TOML
    let config: Config = toml::from_str(&*config_contents).unwrap();

    let start_html = format!("<!DOCTYPE html><html lang=\"{}\"><head><title>{}</title></head><body>", config.language, config.title);
    let end_html = "</body></html>";

    let mut full_html_output = start_html;
    full_html_output.push_str(&*html_output);
    full_html_output.push_str(end_html);

    // Write html_output to file
    fs::write(out_filename, full_html_output).expect("Unable to write file");

    Ok(())
}
