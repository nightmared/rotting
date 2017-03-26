pub use std::ffi::{CStr, CString};
pub use libc::calloc;
pub use std::{mem, ptr};
pub use libc::{c_int, c_void, c_long, c_char, c_uint, c_ulonglong};

// Expose fields from the representation by converting char* to String
#[macro_export]
macro_rules! fn_expose_field {
    ($field:ident, $name:ident) => {
        pub fn $name<'a>(&self) -> &'a str {
            char_to_str(unsafe {(*self.repr).$field })
        }
    };
    ($field:ident) => {fn_expose_field!($field, $field);};
}

#[macro_export]
macro_rules! fn_bool_expose_field {
    ($name:ident) => {
        pub fn $name(&self) -> bool {
            unsafe {
                ((*self.repr).$name)(self.repr) == 1
            }
        }
    };
}

pub fn char_to_str<'a>(char: *const c_char) -> &'a str {
    if char.is_null() {
        ""
    } else {
        let slice = unsafe { CStr::from_ptr(char) };
        let str = slice.to_str().unwrap_or("").clone();
        drop(slice);
        str
    }
}

pub fn str_to_char(name: &str) -> *mut c_char {
    let cstr = CString::new(name).unwrap();
    let len = name.len() + 1;
    unsafe {
        let buf: *mut c_char = calloc(len, mem::size_of::<c_char>()) as *mut c_char;
        if buf.is_null() {
            panic!(format!("calloc({}, {}) failed !", len, mem::size_of::<c_char>()));
        }
        let ptr = cstr.into_raw();
        // no need to add a NULL byte at the end of the bufgfer because it's already allocated to 0
        // (calloc)
        ptr::copy_nonoverlapping(ptr, buf, len);
        // take back ownership of the raw pointer to prevent leaking memory
        let tmp = CString::from_raw(ptr);
        drop(tmp);
        buf
    }
}
