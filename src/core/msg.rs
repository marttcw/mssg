use yansi::Paint;

pub fn help() {
    print!("mssg - Mart's Static Site Generator\n\
            \n\
            init Initialize a new site project\n\
            gen  Generate the static site\n\
            help Display this help message\n");
}

pub fn err_handle(msg: &'static str, print_help: bool) {
    eprintln!("{}: {}", Paint::red("ERROR").bold(), msg);
    if print_help == true {
        self::help();
    }
}


