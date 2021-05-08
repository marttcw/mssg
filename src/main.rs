mod blog;
mod cli;
mod files;
mod md;

use std::io::Result;

fn main() -> Result<()> {
    match cli::parse().subcmd {
        cli::SubCommand::Build(b) => {
            let mut pp: files::PrimaryPaths =
                files::PrimaryPaths::new(&b.source_directory, &b.destination_directory);

            pp.paths_check()?;
            pp.traverse()?;
            pp.build()?;
        }
        cli::SubCommand::Blog(mut b) => {
            if b.src_dir == None {
                b.src_dir = Some(String::from("src/blog"));
            }

            if b.list {
                let entries = blog::get_list(&b.src_dir.unwrap())?;
                for entry in entries {
                    println!("{}", entry.to_list_entry());
                }
            } else if b.new {
                blog::new(&b.src_dir.unwrap())?;
            } else {
            }
        }
    }
    Ok(())
}
