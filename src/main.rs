mod cli;
mod files;
mod md;
mod blog;

use std::io::Error;

fn main() -> Result<(), Error> {
    match cli::parse().subcmd {
        cli::SubCommand::Build(b) => {
            let mut pp: files::PrimaryPaths = files::PrimaryPaths::new(&b.source_directory,
                &b.destination_directory);

            pp.paths_check()?;
            pp.traverse()?;
            pp.build()?;
        },
        cli::SubCommand::Blog(b) => {
            if b.list {
                let entries = blog::get_list(&b.source_directory)?;
                for entry in entries {
                    println!("{}", entry.to_list_entry());
                }
            } else if b.new {
                blog::new(&b.source_directory)?;
            }
        },
    }
    Ok(())
}

