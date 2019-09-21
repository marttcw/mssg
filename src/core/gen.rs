use std::{fs, env, io::BufReader, io::prelude::*, path::Path};
use pulldown_cmark::{Parser, Options, html};
use toml::Value;

// Generate the site
pub fn gen() -> std::io::Result<()> {
    // Set base directories
    let _gen_dir = Path::new("build");
    let _src_dir = Path::new("src");

    // Go to src directory
    assert!(env::set_current_dir(&_src_dir).is_ok());
    markdown("src/index.md")?;

    Ok(())
}

fn markdown(filename: &str) -> std::io::Result<()> {
    let mut html_output = String::new();

    // Open file and read it to string
    let file = fs::File::open(filename)?;
    let mut buf_reader = BufReader::new(file);
    let mut contents = String::new();
    buf_reader.read_to_string(&mut contents)?;

    // Parse the markdown contents to HTML
    let mut options = Options::empty();
    options.insert(Options::ENABLE_STRIKETHROUGH);
    let parser = Parser::new_ext(&*contents, options);
    html::push_html(&mut html_output, parser);

    print!("{}", html_output);

    Ok(())
}
