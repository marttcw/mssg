use std::{fs, env, io::BufReader, io::prelude::*, path::Path, error::Error};
use walkdir::{DirEntry, WalkDir};
use pulldown_cmark::{Parser, Options, html};
use toml::Value;
use serde::Deserialize;

// Local deserialization structs
#[derive(Deserialize)]
struct Config {
    title: String,
    description: String,
    language: String,
    global: Global_Config,
}

#[derive(Deserialize)]
struct Global_Config {
    order: Vec<String>,
    css: Option<String>,
}

// Local structs
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

fn is_directory(entry: &DirEntry) -> bool {
    return entry.file_type().is_dir();
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

fn nav_generator(html_output: &mut String, base_dir_path: &str) -> std::io::Result<()> {
    html_output.push_str("<nav>");

    let mut fname: String;
    let mut fpath: String;
    let paths = fs::read_dir(base_dir_path).unwrap();

    for path in paths {
        let pa = path.unwrap();
        fpath = pa.path().display().to_string();
        fname = pa.path().file_name().unwrap().to_os_string().into_string().unwrap();
        if fpath != base_dir_path && pa.path().is_dir() {
            html_output.push_str(format!("<a href=\"/{}\">{}</a>", &*fname, &*fname).as_str());
        }
    }

    html_output.push_str("</nav>");
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

    // Navigation output
    let mut nav_output = String::new();
    nav_generator(&mut nav_output, "src")?;

    let mut full_html_output = format!("<!DOCTYPE html><html lang=\"{}\"><head><meta charset=\"utf-8\"><title>{}</title>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        , config.language, config.title);

    // Check if it exists and if so push it to html
    match config.global.css {
        Some(css) => {
            full_html_output.push_str(format!("<link rel=\"stylesheet\" type=\"text/css\" href=\"/{}\">", css.as_str()).as_str());
            fs::copy(format!("src/{}", css), format!("build/{}", css))?;
        }
        None => (),
    }

    full_html_output.push_str("</head><body>");
    full_html_output.push_str(&*nav_output);
    full_html_output.push_str(&*html_output);
    full_html_output.push_str("</body></html>");

    // Write html_output to file
    fs::write(out_filename, full_html_output).expect("Unable to write file");

    Ok(())
}
