use clap::Clap;

#[derive(Clap, Debug)]
#[clap(version = "0.0.3 Alpha", author = "mtcw")]
pub struct Opts {
    #[clap(subcommand)]
    pub subcmd: SubCommand,
}

#[derive(Clap, Debug)]
pub enum SubCommand {
    #[clap(version = "0.0.3 Alpha", author = "mtcw")]
    Build(Build),
}

/// Build your site
#[derive(Clap, Debug)]
pub struct Build {
    /// Verbose output
    #[clap(short, long)]
    pub verbose: bool,

    /// The source directory
    #[clap(short, long, default_value = "src")]
    pub source_directory: String,

    /// The destination directory
    #[clap(short, long, default_value = "dst")]
    pub destination_directory: String,
}

pub fn parse() -> Opts {
    return Opts::parse();
}

