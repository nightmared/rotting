#![feature(const_fn, stmt_expr_attributes, question_mark)]
extern crate libc;
mod ffi;
#[macro_use]
mod helpers;
use helpers::*;
use ffi::*;
use std::fmt;


/// WARNING: NEVER EVER use this code in production, it's not memory-safe AT ALL !

/// Expose the LXC API version
pub fn version<'a>() -> &'a str {
    char_to_str(unsafe { lxc_get_version() })
}

// Ensure that lxc_get_global_config_item run only once, otherwise the path returned might change and break the API
static mut conf_path: *const i8 = ptr::null();

unsafe fn config_path() -> *const c_char {
    if conf_path.is_null() {
        let cstr = CStr::from_bytes_with_nul(b"lxc.lxcpath\0").unwrap();
        conf_path = lxc_get_global_config_item(cstr.as_ptr());
    }
    conf_path
}

// Return the path used to store containers and theirs configurations
pub fn get_config_path<'a>() -> &'a str {
    char_to_str(unsafe { config_path() })
}

pub enum LxcError {
    NullPointer,
    AlreadyExists,
    CreationFailed,
    DropFailed,
    UnknownValueReturned,
    AllocationFailed,
    NameNotSet,
    TemplateNotSet
}

impl fmt::Debug for LxcError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", match *self {
            LxcError::NullPointer => "Null Pointer passed to the function",
            LxcError::AlreadyExists => "The container already exists",
            LxcError::CreationFailed => "The call to <container>.create failed",
            LxcError::DropFailed => "Container reference dropping failed (call to lxc_container_put(<container>) returned 1)",
            LxcError::UnknownValueReturned => "A function returned an unknown value, please report this as a bug !",
            LxcError::AllocationFailed => "Container allocation failed, during call to lxc_container_new",
            LxcError::NameNotSet => "Container name was not set",
            LxcError::TemplateNotSet => "Container template was not set"
        })
    }
}

pub struct ContainerBuilder {
    flags: c_int,
    name: Option<String>,
    bdev_specs: Option<bdev_specs>,
    template: Option<String>,
    args: Vec<*mut c_char>
}

impl ContainerBuilder {
    pub fn new() -> ContainerBuilder  {
        ContainerBuilder {
            flags: 0,
            name: None,
            bdev_specs: None,
            template: None,
            args: vec![]
        }
    }

    fn is_ok(&self) -> Result<(), LxcError> {
        if self.name == None {
            Err(LxcError::NameNotSet)
        } else if self.template == None { 
            Err(LxcError::TemplateNotSet)
        } else {
            Ok(())
        }
    }

    // Set verbosity (true imply verbose (normal mode) and false set the LXC_CREATE_QUIET flag to be
    // passed to <container>.create.
    pub fn set_verbose(mut self, cond: bool) ->  ContainerBuilder {
        self.flags = match cond {
            true => 0,
            false => CREATE_QUIET
        };
        self
    }

    pub fn set_name(mut self, name: & str) -> ContainerBuilder {
        self.name = Some(name.into());
        self
    }

    pub fn set_backend_device(mut self, bs: bdev_specs) -> ContainerBuilder {
        self.bdev_specs = Some(bs);
        self
    }

    pub fn set_template(mut self, template: &str) -> ContainerBuilder {
        self.template = Some(template.into());
        self
    }

    pub fn set_args(mut self, args: &[&str]) -> ContainerBuilder {
        self.args = args
            .iter()
            .map(|x| str_to_char(x))
            .collect();
        self
    }

    pub fn add_args(mut self, arg: &str) -> ContainerBuilder {
        self.args.push(str_to_char(arg));
        self
    }

    pub fn create(mut self) -> Result<Container, LxcError> {
        if self.is_ok().is_err() {
            return Err(self.is_ok().err().unwrap());
        };
        // no bdev_specs ? let's set the backend to "best"
        if self.bdev_specs.is_none() {
            let mut bs: bdev_specs = unsafe { mem::zeroed() };
            bs.fstype = str_to_char("best") as *mut _;
            self.bdev_specs = Some(bs);
        }

        let mut c = Container::new(&(self.name.unwrap())).unwrap();
        c.create(self.flags, &self.template.unwrap(), &mut self.bdev_specs.unwrap(), self.args)?;
        return Ok(c);
    }
}

/// The public "stable" API to create, edit and query containers.
/// It's just a wrapper around a `struct lxc_container`. 
#[derive(Clone)]
pub struct Container {
    repr: *mut container
}

impl fmt::Debug for Container {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let str = self.name();
        write!(f, "Container {}", &str)
    }
}

impl Container {
    // create a new structure but doesn't instanciate a container, because it doesn't have enought
    // informations
    pub fn new(name: &str) -> Result<Container, LxcError> {
       let c = unsafe { lxc_container_new(str_to_char(name), ptr::null()) };
       if c.is_null() {
            Err(LxcError::AllocationFailed)
       } else {
            Container::from_raw(c) 
       } 
    }

    /// Create a new container based on its representation in C
    pub fn from_raw(c: *mut container) -> Result<Self, LxcError> {
        if c.is_null() {
            Err(LxcError::NullPointer)
        } else {
            Ok(Container {
                repr: c
            })
        }
    }

    pub fn release(&self) -> Result<(), LxcError> {
        let ret = unsafe { lxc_container_put(self.repr) };
        match ret { 
            -1 => Err(LxcError::DropFailed),
            0 | 1  => Ok(()),
            _  => Err(LxcError::UnknownValueReturned)
        }
    }

    pub fn create(&mut self, flags: c_int, template: &str, bdev: &mut bdev_specs, args: Vec<*mut c_char>) -> Result<(), LxcError> {
        if self.is_defined() {
            let tmp = self.release();
            if tmp.is_err() {
                return tmp;
            }
            return Err(LxcError::AlreadyExists);
        }
        if unsafe { ((*self.repr).create)(self.repr, str_to_char(template), bdev.fstype, bdev, flags, args.as_ptr())} == false as u8 {
            let _ = self.release();
            return Err(LxcError::CreationFailed);
        }
        Ok(())
    }

    fn_expose_field!(name);
    fn_expose_field!(configfile, config_file);
    fn_expose_field!(config_path);
    fn_expose_field!(pidfile, pid_file);
    fn_bool_expose_field!(is_defined);
    fn_bool_expose_field!(is_running);
}


pub fn list_all() -> Option<Vec<Container>> {

    let mut c: *mut container = unsafe { &mut mem::zeroed() } as *mut container;
    let mut list = &mut c as *mut _;

    let nb = unsafe { list_all_containers(config_path(), ptr::null_mut(), &mut list) };
    if nb <= 0 {
        return None
    }
    Some(
        unsafe { Vec::from_raw_parts(list, nb as usize, nb as usize) }
            .iter()
            .map(|x| Container::from_raw(*x).unwrap())
            .collect()
    )
}

pub fn list_running() -> Option<Vec<Container>> {
    match list_all() {
        Some(x) => Some(
            x.iter()
                .filter(|x| x.is_running())
                .cloned()
                .collect()
        ),
        None => None
    }
}
