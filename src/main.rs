mod cli;
mod files;
mod md;

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
    }
    Ok(())
}

