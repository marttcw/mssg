use std::{fs, env, io::BufReader, io::prelude::*, path::Path};
use pulldown_cmark::{Parser, Options, html};
use toml::Value;

// Generate the site
pub fn gen() -> std::io::Result<()> {
    markdown("src/index.md", "build/index.html")?;

    Ok(())
}

/*
fn list_dirs(_list: Vec<String>) -> std::io::Result<()> {
    // Set base directories
    let _gen_dir = Path::new("build");
    let _src_dir = Path::new("src");

    Ok(())
}
*/

fn markdown(in_filename: &str, out_filename: &str) -> std::io::Result<()> {
    let mut html_output = String::new();

    // Open file and read it to string
    let contents = fs::read_to_string(in_filename).expect("Unable to read file");

    // Parse the markdown contents to HTML
    let mut options = Options::empty();
    options.insert(Options::ENABLE_STRIKETHROUGH);
    let parser = Parser::new_ext(&*contents, options);
    html::push_html(&mut html_output, parser);

    // Write html_output to file
    fs::write(out_filename, html_output).expect("Unable to write file");

    Ok(())
}
