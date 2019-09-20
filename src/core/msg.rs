use yansi::Paint;

pub fn help() {
    print!("mssg - Mart's Static Site Generator\n\
            \n\
            new  Create a new site project\n\
            init Create a new site project on the current directory\n\
            gen  Generate the static site\n\
            help Display this help message\n");
}

pub fn err_handle(msg: &'static str) {
    eprintln!("{}: {}", Paint::red("ERROR").bold(), msg);
    self::help();
}


