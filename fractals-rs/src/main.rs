extern crate gl;
extern crate glutin;
extern crate libc;


mod gui;

use gui::Gui;
use std::sync::mpsc::channel;
use std::sync::{Mutex, Arc};
use std::thread;

/*        if gl::BindVertexArray.is_loaded() {
            let mut vao = mem::uninitialized();
            gl::GenVertexArrays(1, &mut vao);
            gl::BindVertexArray(vao);
        }

        let pos_attrib = gl::GetAttribLocation(program, b"position\0".as_ptr() as *const _);
        let color_attrib = gl::GetAttribLocation(program, b"color\0".as_ptr() as *const _);
        gl::VertexAttribPointer(pos_attrib as gl::types::GLuint, 2, gl::FLOAT, 0,
                                    5 * mem::size_of::<f32>() as gl::types::GLsizei,
                                    ptr::null());
        gl::VertexAttribPointer(color_attrib as gl::types::GLuint, 3, gl::FLOAT, 0,
                                    5 * mem::size_of::<f32>() as gl::types::GLsizei,
                                    (2 * mem::size_of::<f32>()) as *const () as *const _);
        gl::EnableVertexAttribArray(pos_attrib as gl::types::GLuint);
        gl::EnableVertexAttribArray(color_attrib as gl::types::GLuint);

*/
const SIZE: usize = 400;

struct Automate {
    data: Arc<Mutex<Vec<f32>>>,
    temp: Vec<f32>
}

macro_rules! custom_add {
    ($val:expr) => {
        $val
    };
    ($val:expr, $($e:expr),*) => {{
        $val + custom_add!($($e),*)
    }};
}

impl Automate {
    fn new() -> Automate {
        let obj = Automate {
            data: Arc::new(Mutex::new(vec![0.; SIZE])),
            temp:  vec![0.; SIZE]
        };
        let lock = obj.data.clone();
        let mut data = lock.lock().unwrap();
		(*data)[SIZE/2] = 1.;
        obj
    }
    fn next(&mut self) {
        let lock = self.data.clone();
		let mut data = lock.lock().unwrap();
        let copy: &[f32] = &*data.clone();
        copy.iter().enumerate().fold(copy.get(0).unwrap(), 
			|previous, (pos, value)| {
                self.temp[pos] = if custom_add!(previous, value, copy.get(pos+1).unwrap_or(&0.)) > 0. { 1. } else { 0. };
                value
         });
        *data = self.temp.clone();
    }
    fn data(&self) -> Vec<f32> {
        let lock = self.data.clone();
		let data = lock.lock().unwrap();
        (*data).clone()
    }
}

fn main() {
    let mut to_concat: Vec<f32> = Vec::with_capacity(400);
    let hsize = (SIZE/2) as i32;
    for i in -hsize..hsize {
        to_concat.push(i as f32/hsize as f32);
    }

	let window = glutin::Window::new().unwrap();
    unsafe { window.make_current() };
    gl::load_with(|symbol| window.get_proc_address(symbol) as *const _);

    let mut gui = Gui::new();
    
    let mut auto = Automate::new();
    let mut data: Vec<f32> = Vec::with_capacity(SIZE*2);

    for event in window.wait_events() {
        auto.next();
        data = to_concat.iter().zip(auto.data().into_iter()).filter(|x| x.1 == 1.).flat_map(|s| vec![*s.0, s.1].into_iter()).collect();
        gui.redraw(&*data);
        window.swap_buffers();

        match event {
            glutin::Event::Closed => break,
            _ => ()
        }
    }
}
