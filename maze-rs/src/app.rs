use std::{mem, ptr, thread, io, slice};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::ffi::CString;
use std::os::raw::c_void;
use libc;
use libc::{malloc, free, calloc};
use std::io::prelude::*;
use std::fs::File;
use std::path::Path;
use std::ops::{Index, IndexMut};


use gl;
use gl::types::*;

use wayland_client::wayland::get_display;
use wayland_client::wayland::compositor::WlCompositor;
use wayland_client::wayland::shell::WlShell;
use wayland_client::wayland::shm::{WlShm, WlShmFormat};


/*
 * fn delete_program(prog: Option<&AtomicUsize>) {
 *     if prog.is_some() {
 *         let n = prog.unwrap().load(Ordering::Relaxed) as u32;
 *         unsafe { gl::DeleteProgram(n); }
 *     }
 * }
 * 
 * fn delete_shader(shad: Option<&AtomicUsize>) {
 *     if shad.is_some() {
 *         let n = shad.unwrap().load(Ordering::Relaxed) as u32;
 *         unsafe { gl::DeleteShader(n); }
 *     }
 * }
 * 
 * #[inline]
 * fn delete_buffer(addr: &mut GLuint) {
 *     if *addr != 0 {
 *         unsafe { gl::DeleteBuffers(1, addr as *const u32); }
 *         *addr = 0;
 *     }
 * }
 * 
 * #[inline]
 * fn delete_vertex_array(addr: &mut GLuint) {
 *     if *addr != 0 {
 *         unsafe { gl::DeleteVertexArrays(1, addr as *const u32); }
 *         *addr = 0;
 *     }
 * }
 */

#[derive(Debug)]
pub enum AppError {
    ConnectionFailed,
    ShaderCompilationFailed,
    UnknowShader,
    MissingShader,
    LinkingFailed,
    ThreadPanicked,
    ConnectionClosed,
    BufferNotSwapped,
    NotLinked,
    GLContextError,
    IOError(io::ErrorKind),
    BufferFull
}

pub enum Event {
    Closed,
    Unknown
}


impl From<io::Error> for AppError {
    fn from(err: io::Error) -> AppError {
        AppError::IOError(err.kind())
    }
}

pub struct App {
    win: Window,
    fshad: Option<AtomicUsize>,
    vshad: Option<AtomicUsize>,
    prog: Option<AtomicUsize>,
    vao: GLuint,
    vbo: GLuint,
    data: *mut GLfloat,
    len: usize
}

impl App {
    pub fn new() -> Result<App, AppError> {
        wayland_env!(WaylandEnv,
            compositor: WlCompositor,
            shell: WlShell,
            shm: WlShm
        );

        let (display, iter) = match get_display() {
            Ok(d) => d,
            Err(e) => panic!("Unable to connect to a wayland compositor: {:?}", e)
        };

        let (env, mut evt_iter) = WaylandEnv::init(display, iter);

        let compositor = env.compositor.as_ref().map(|o| &o.0).unwrap();
        let shell = env.shell.as_ref().map(|o| &o.0).unwrap();
        let shm = env.shm.as_ref().map(|o| &o.0).unwrap();

        let surface = compositor.create_surface();
        let shell_surface = shell.get_shell_surface(&surface);

        Ok(App {
            win: win,
            fshad: None,
            vshad: None,
            prog: None,
            vao: 0,
            vbo: 0,
            data: ptr::null_mut(),
            len: 0
        })
    }

    pub fn load_gl(&self) {
        if !self.win.is_current() {
            gl::load_with(|symbol| self.win.get_proc_address(symbol) as *const _);
            unsafe { self.win.make_current() }.unwrap();
        }

    }

    #[inline]
    pub fn show(&mut self) {
        if !self.win.is_current() {
            self.load_gl();
        }
        self.win.show();
    }

    #[inline]
    pub fn set_title(&mut self, title: &str) {
            self.win.set_title(title);
    }
/*
 *    pub fn add_shader(&mut self, src: &str, ty: GLenum) -> Result<GLuint, AppError> {
 *         let shader;
 *         unsafe {
 *             shader = gl::CreateShader(ty);
 * 
 *             let c_str = CString::new(src.as_bytes()).unwrap();
 *             gl::ShaderSource(shader, 1, &c_str.as_ptr(), ptr::null());
 *             gl::CompileShader(shader);
 * 
 *             let mut status = gl::FALSE as GLint;
 *             gl::GetShaderiv(shader, gl::COMPILE_STATUS, &mut status);
 * 
 *             if status != (gl::TRUE as GLint) {
 *                 return Err(AppError::ShaderCompilationFailed);    
 *             }
 *         }
 * 
 *         if ty == gl::FRAGMENT_SHADER {
 *             delete_shader(self.fshad.as_ref());
 *             self.fshad = Some(AtomicUsize::new(shader as usize));
 *         } else if ty == gl::VERTEX_SHADER {
 *             delete_shader(self.vshad.as_ref());
 *             self.vshad = Some(AtomicUsize::new(shader as usize));
 *         } else {
 *             return Err(AppError::UnknowShader);
 *         }
 *         Ok(shader)
 *     }
 * 
 *     pub fn load_shader(&mut self, file: &Path, ty: GLenum) -> Result<GLuint, AppError> {
 *         let mut f = try!(File::open(file));
 *         let mut s = String::new();
 *         try!(f.read_to_string(&mut s));
 *         self.add_shader(&s, ty)
 *     }
 * 
 *     pub fn link(&mut self) -> Result<GLuint, AppError> {
 *         if self.fshad.is_none() || self.vshad.is_none() {
 *             return Err(AppError::MissingShader);
 *         }
 * 
 *         let mut status = gl::FALSE as GLint;
 *         let program = unsafe {
 *             let program = gl::CreateProgram();
 * 
 *             gl::AttachShader(program, self.vshad.as_ref().unwrap().load(Ordering::Relaxed) as u32);
 *             gl::AttachShader(program, self.fshad.as_ref().unwrap().load(Ordering::Relaxed) as u32);
 *             gl::LinkProgram(program);
 * 
 *             gl::GetProgramiv(program, gl::LINK_STATUS, &mut status);
 *             program
 *         };
 * 
 *         if status != (gl::TRUE as GLint) {
 *             return Err(AppError::LinkingFailed);
 *         }
 *         // Install the program in the `prog` field
 *         delete_program(self.prog.as_ref());
 *         self.prog = Some(AtomicUsize::new(program as usize));
 *         Ok(program)
 *     }
 * 
 *     // Unsafe boundary needed here ?
 *     pub fn draw(&mut self, data: &[GLfloat]) -> Result<(), AppError> {
 *         unsafe {
 *             delete_vertex_array(&mut self.vao);
 *             gl::GenVertexArrays(1, &mut self.vao);
 *             gl::BindVertexArray(self.vao);
 *             self.set_data(data);
 *             if self.prog.is_none() {
 *                 return Err(AppError::NotLinked);
 *             }
 *             let program = self.prog.as_ref().unwrap().load(Ordering::Relaxed) as u32;
 *             gl::UseProgram(program);
 *             gl::BindFragDataLocation(program, 0,
 *                  CString::new("out_color").unwrap().as_ptr());
 *             let pos_attr = gl::GetAttribLocation(program,
 *                 CString::new("position").unwrap().as_ptr());
 *             gl::EnableVertexAttribArray(pos_attr as GLuint);
 *             gl::VertexAttribPointer(pos_attr as GLuint, 2, gl::FLOAT,
 *                 gl::FALSE as GLboolean, 0, ptr::null());
 *         }
 *         self.redraw()
 *     }
 * 
 *     pub unsafe fn set_data(&mut self, data: &[GLfloat]) {
 *         if self.len != 0 && self.data as usize != 0 {
 *             free(self.data as *mut libc::c_void);
 *         }
 *         self.len = data.len();
 *         let len = self.len * mem::size_of::<GLfloat>();
 *         self.data = malloc(len) as *mut GLfloat;
 *         ptr::copy(data.as_ptr(), self.data, self.len);
 *         delete_buffer(&mut self.vbo);
 *         gl::GenBuffers(1, &mut self.vbo);
 *         gl::BindBuffer(gl::ARRAY_BUFFER, self.vbo);
 *         gl::BufferData(gl::ARRAY_BUFFER,
 *             len as GLsizeiptr,
 *             self.data as *const c_void,
 *             gl::DYNAMIC_DRAW);
 *     }
 * 
 *     pub fn redraw(&self) -> Result<(), AppError> {
 *         let (w, h) = self.win.get_inner_size().unwrap();
 *         self.win.set_inner_size(w, h);
 * 
 *         unsafe {
 *             gl::ClearColor(1., 0., 0., 1.0);
 *             gl::Clear(gl::COLOR_BUFFER_BIT);
 * 
 *             // Draw a triangle from the 3 vertices
 *             gl::DrawArrays(gl::TRIANGLES, 0, (self.len/2) as i32);
 *         }
 * 
 *         // TODO: tedious to write, should be possible to write it better using 
 *         // the `From` trait declared above
 *         match self.win.swap_buffers() {
 *             Ok(()) => Ok(()),
 *             Err(e) => Err(AppError::GLContextError(e))
 *         }
 *     }
 * 
 * 
 *     #[inline]
 *     pub fn next_event(&self) -> Event {
 *         match self.win.wait_events().next() {
 *             Some(glutin::Event::Closed) => Event::Closed,
 *             Some(_) => Event::Unknown,
 *             None => panic!("wait_event shouldn't return None")
 *         }
 *     }
 *     
 *     // TODO: get rid of the 'static
 *     pub unsafe fn exec<T, F>(&self, f: F) -> Result<T, AppError> 
 *         where F: FnOnce() -> T + Send + 'static,
 *         T: Send + 'static {
 *         let child = thread::spawn(f);
 *         match child.join() {
 *             Ok(x) => Ok(x),
 *             Err(_) => Err(AppError::ThreadPanicked)
 *         }
 *     }
 *
 * }
 *
 * impl Iterator for App {
 *     type Item = Event;
 * 
 *     fn next(&mut self) -> Option<Event> {
 *         let poll = self.win.poll_events().next();
 *         if poll.is_none() {
 *             None
 *         } else {
 *             match poll.unwrap() {
 *                 glutin::Event::Closed => Some(Event::Closed),
 *                 _ => Some(Event::Unknown)
 *             }
 *         } 
 *     }
 * }
 * 
 * impl Drop for App {
 *     fn drop(&mut self) {
 *         delete_program(self.prog.as_ref());
 *         delete_shader(self.fshad.as_ref());
 *         delete_shader(self.vshad.as_ref());
 *         delete_buffer(&mut self.vbo);
 *         delete_vertex_array(&mut self.vao);
 *     }
 * }
 */
pub struct Grid {
    data: *mut GLfloat,
    size: usize,
    len: usize
}

impl Grid {
    pub fn new(size: usize) -> Grid {
        // a single rectangle takes 12 GLfloat
        let data = unsafe { calloc(0, size * size * mem::size_of::<GLfloat>() * 12)}; 
        Grid {
            data: data as *mut GLfloat,
            size: size * size * 12,
            len: 0
        }
    }

    pub fn as_array<'a>(&self) -> &'a[GLfloat] {
        unsafe {
            slice::from_raw_parts(self.data, self.len)
        }
    }

    pub fn push(&mut self, val: &[GLfloat]) -> Result<(), AppError> {
        if self.len + val.len() <= self.size {
            unsafe { ptr::copy_nonoverlapping(val.as_ptr(), self.data, val.len()); }
            self.len += val.len();
            Ok(())
        } else {
            Err(AppError::BufferFull)
        }
    }
}

// maybe implement that latter, but I would need to check whether or not a rectangle should be generated for
// each set of 6 vertices (aka 12 GLfloat)
/*impl Index<usize> for Grid {
    type Output = [GLfloat];

    fn index(&self, index: usize) -> &[GLfloat] {
        unsafe {
            let base = self.data.offset((self.width * index * 12) as isize);
            slice::from_raw_parts(base, self.width)
        }
    }
}

impl IndexMut<usize> for Grid {
    fn index_mut(&mut self, index: usize) -> &mut [GLfloat] {
        unsafe {
            let base = self.data.offset((self.width * index * 12) as isize);
            slice::from_raw_parts_mut(base, self.width)
        }
    }
}*/
