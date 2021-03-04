use clap::Clap;

#[derive(Clap, Debug)]
#[clap(version = "0.0.3 Alpha", author = "mtcw <mtcw@disroot.org>")]
pub struct Opts {
    #[clap(subcommand)]
    pub subcmd: SubCommand,
}

#[derive(Clap, Debug)]
pub enum SubCommand {
    #[clap(version = "0.0.3 Alpha", author = "mtcw <mtcw@disroot.org>")]
    Build(Build),
}

/// Build your site
#[derive(Clap, Debug)]
pub struct Build {
    /// Verbose output
    #[clap(short, long)]
    pub verbose: bool,
}

pub fn parse() -> Opts {
    return Opts::parse();
}

