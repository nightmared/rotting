extern crate lxc;

fn main() {
    println!("LXC-rs demo\n-----------\n");
    println!("The current LXC path is {}", lxc::get_config_path());
    let arr = lxc::list_all().unwrap_or(vec![]);
    println!("There is currently {} containers on the system", arr.len());
    if arr.len() > 0 {
        for i in arr {
            println!("\t- {} - {}", &i.name(), &i.config_file());
        }
        print!("Of which ");
        let arr = lxc::list_running().unwrap_or(vec![]);
        if arr.len() > 0 {
            println!("the following ones are running:");
            for i in arr {
                println!("\t- {}", &i.name());
            }
        } else {
            println!("none is running");
        }
    }
    let c = lxc::ContainerBuilder::new()
        .set_name("df")
        .set_template("download")
        .set_args(&["-d", "ubuntu", "-a", "amd64", "-r", "yakkety"])
        .create();
    println!("{:?}", c);
}
