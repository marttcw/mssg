mod cli;
mod files;

use std::io::Error;

fn main() -> Result<(), Error> {
    match cli::parse().subcmd {
        cli::SubCommand::Build(b) => {
            let mut pp: files::PrimaryPaths = files::PrimaryPaths::new(&b.source_directory,
                &b.destination_directory);

            if pp.paths_check()? {
                println!("Paths OK");
            } else {
                eprintln!("Paths not found");
            }
        },
    }
    Ok(())
}

