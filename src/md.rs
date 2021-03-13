use std::io::Error;
use std::fs::read_to_string;
use pulldown_cmark::{Parser, Options, html};

pub fn convert_to_html(path: &str) -> Result<String, Error> {
    let in_file_str = read_to_string(path)?;

    let mut options = Options::empty();
    options.insert(Options::ENABLE_STRIKETHROUGH);
    options.insert(Options::ENABLE_TABLES);
    options.insert(Options::ENABLE_FOOTNOTES);
    options.insert(Options::ENABLE_TASKLISTS);

    let parser = Parser::new_ext(&in_file_str, options);

    let mut html_output = String::new();
    html::push_html(&mut html_output, parser);

    Ok(html_output)
}

