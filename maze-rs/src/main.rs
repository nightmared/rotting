extern crate gl;
#[macro_use]
extern crate wayland_client;
extern crate libc;

mod app;
use app::{App, Event, Grid};
use gl::types::*;
use std::env;
use std::thread;
use std::time::Duration;

// Generate two triangles to render the square
fn gen_rect(x: f32, y: f32, w: f32, h: f32) -> [GLfloat; 12] {
	[
		x,		y,
		x+w,	y,
		x+w,	y-h,
		x+w,	y-h,
		x,		y-h,
		x,		y
	]
}

#[inline]
fn gen_square(x: f32, y: f32, side: f32) -> [GLfloat; 12] {
	gen_rect(x, y, side, side)
}

fn main() {
	let mut app = App::new().unwrap();
	app.set_title("maze");
	app.load_gl();

	// TODO: get real (absolute) shaders location
	let mut path = env::current_exe().unwrap();
	path.pop();
	path.push("..");
	path.push("..");
	path.push("src");
	path.push("shaders");
	path.push("main.vert");
    app.load_shader(path.as_path(), gl::VERTEX_SHADER).unwrap();
	path.pop();
	path.push("main.frag");
    app.load_shader(path.as_path(), gl::FRAGMENT_SHADER).unwrap();
    app.link().unwrap();
	/*let mut grid = Grid::new(5);

	for i in 0..5 {
		for j in 0..5 {
			if j%2 == 0 {
                let square = gen_square(0.2*i as f32, 0.2*j as f32, 0.2);
                println!("{:?}", square);
                grid.push(&square).unwrap();
			}
		}
	}
    let arr = grid.as_array();
    println!("{:?}", arr);
    app.draw(&vertices).unwrap();*/
	app.show();
    let mut i = 0.;
    loop {
        if let Some(x) = app.next() {
            match x {
                Event::Closed => break,
                _ => {}
            }
        }
        app.draw(&gen_square(i, 0., 0.2)).unwrap();
        app.redraw();
        i += 0.1;
    }
}
