extern crate gl;
extern crate glutin;
extern crate libc;

use gl::types::*;
use std::fs::File;
use std::io::Read;
use std::ffi::CString;
use std::ptr;
use std::mem;
use std::os::unix::fs::MetadataExt;


fn shader_cc<'a>(fname: &'a str, shader_type: GLenum) -> Result<GLuint, String> {
	// read shader
    let mut f = File::open(fname).unwrap();
	let f_size = f.metadata().expect("Couldn't read metadata on the shader !").size();
    let mut buffer = Vec::with_capacity(f_size as usize);
    f.read_to_end(&mut buffer).unwrap();

	// Compile it
	let mut test: GLint = gl::FALSE as i32;
	let shader: GLuint;
    unsafe {
	    shader = gl::CreateShader(shader_type);
	    gl::ShaderSource(shader, 1, &CString::new(buffer).unwrap().as_ptr(), ptr::null());
	    gl::CompileShader(shader);
		// has compilation failed ?
    	gl::GetShaderiv(shader, gl::COMPILE_STATUS, &mut test);
    }

	if test != gl::TRUE as i32 {
		return Err(format!("Shader compilation failed"));
	}

    Ok(shader)
}

fn new_prog<'a>(path_vs: &'a str, path_fs: &'a str) -> GLuint {
	let vs: GLuint = shader_cc(path_vs, gl::VERTEX_SHADER).expect("vertex shader should compile");
	let fs: GLuint = shader_cc(path_fs, gl::FRAGMENT_SHADER).expect("fragment shader should compile");

	let program: GLuint;
	unsafe {
		program = gl::CreateProgram();

		gl::AttachShader(program, vs);
		gl::AttachShader(program, fs);

		gl::DeleteShader(vs);
		gl::DeleteShader(fs);

		gl::LinkProgram(program);
		gl::UseProgram(program);
	}
	program
}


pub struct Gui {
	pub vao: GLuint,
	vbo: GLuint,
    prog: GLuint
}

impl Gui {
    pub fn new() -> Gui {
        let shader_prog = new_prog("./src/shaders/vertex.shader", "./src/shaders/fragment.shader");
        Gui { vao: 0, vbo: 0, prog: shader_prog }
    }

    pub fn redraw(&mut self, data: &[f32]) {
        unsafe {
            gl::GenVertexArrays(1, &mut self.vao);
            gl::BindVertexArray(self.vao);
            gl::GenBuffers(1, &mut self.vbo);
            gl::BindBuffer(gl::ARRAY_BUFFER, self.vbo);
            gl::BufferData(gl::ARRAY_BUFFER,
                (data.len() * mem::size_of::<f32>()) as GLsizeiptr,
                data.as_ptr() as *const _,
                gl::STATIC_DRAW);
            
            let position_attribute = gl::GetAttribLocation(self.prog, b"position\0".as_ptr() as *const _) as u32;

            gl::VertexAttribPointer(position_attribute, 2, gl::FLOAT, gl::FALSE, 0, ptr::null());

            gl::EnableVertexAttribArray(position_attribute);

            gl::Enable(gl::PROGRAM_POINT_SIZE);
            gl::ClearColor(0.0, 0.0, 1.0, 1.0);
			gl::Clear(gl::COLOR_BUFFER_BIT);
			gl::BindVertexArray(self.vao);
			gl::DrawArrays(gl::POINTS, 0, data.len() as i32);

        }
    }
}

