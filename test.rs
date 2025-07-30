use libc;
use nix::sys::ioctl;
use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::mem;
use std::os::fd::{AsRawFd, RawFd};
use std::process;

pub struct RgxArg {
    pub pattern: *const c_char,
    pub pattern_len: u64,
}

fn main() {
    let file_path = "/dev/xstrike";
    let mut file = match OpenOptions::new()
        .read(true)
        .write(true)
        .create(false)
        .open(file_path)
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("test-dev: Failed to open {} with {}", file_path, e);
            process::exit(1);
        }
    };
    let fd: RawFd = file.as_raw_fd();

    let mut arg = RgxArg::default();
    let patt = "Ipsum";

    arg.pattern_len = patt.len() as u64;
    arg.pattern = patt;

    let write_data = b"Hello world from the rust to kernel driver\n";
    match file.write_all(write_data) {
        Ok(_) => {}
        Err(e) => eprintln!("test-dev WE: Error writing to device : {}", e),
    }

    let mut read_data = vec![0; 2048];
    match file.read(&mut read_data) {
        Ok(bytes_read) => match String::from_utf8(read_data[..bytes_read].to_vec()) {
            Ok(s) => print!("test-dev R: {}", s),
            Err(e) => eprintln!("test-dev RE: {} {:?}", e, read_data),
        },
        Err(e) => eprintln!("Error reading from device : {}", e),
    }
}
