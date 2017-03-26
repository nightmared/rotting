#![allow(dead_code, non_camel_case_types)]
use helpers::*;

pub type c_bool = u8;


#[repr(u32)]
#[derive(Copy, Clone, Debug)]
pub enum attach_env_policy {
    ATTACH_KEEP_ENV,
    ATTACH_CLEAR_ENV
}

pub static ATTACH_MOVE_TO_CGROUP: c_int = 0x00000001;
pub static ATTACH_DROP_CAPABILITIES: c_int = 0x00000002;
pub static ATTACH_SET_PERSONALITY: c_int = 0x00000004;
pub static ATTACH_LSM_EXEC: c_int = 0x00000008;

pub static ATTACH_REMOUNT_PROC_SYS: c_int = 0x00010000;
pub static ATTACH_LSM_NOW: c_int = 0x00020000;
pub static ATTACH_DEFAULT: c_int = 0x0000FFFF;

//pub static ATTACH_LSM: c_int =  (ATTACH_LSM_EXEC | ATTACH_LSM_NOW);

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct attach_options {
    attach_flags: c_int,
    namespaces: c_int,
    personality: c_long,
    initial_cwd: *mut c_char,
    uid: c_uint,
    gid: c_uint,
    env_policy: attach_env_policy,
    extra_env_vars: *mut *mut c_char,
    extra_keep_env: *mut *mut c_char,
    stdin_fd: c_int,
    stdout_fd: c_int,
    stderr_fd: c_int
}

impl Default for attach_options {
    fn default() -> Self {
        attach_options {
            attach_flags: ATTACH_DEFAULT,
            namespaces: -1,
            personality: -1,
            initial_cwd: ptr::null_mut(),
            uid: c_uint::max_value(),
            gid: c_uint::max_value(),
            env_policy: attach_env_policy::ATTACH_KEEP_ENV,
            extra_env_vars: ptr::null_mut(),
            extra_keep_env: ptr::null_mut(),
            stdin_fd: 0,
            stdout_fd: 1,
            stderr_fd: 2
        }
    }
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct attach_command {
    program: *mut *mut c_char,
    argv: *mut *mut c_char
}

pub static CLONE_KEEPNAME: c_int = (1 << 0);
pub static CLONE_KEEPMACADDR: c_int = (1 << 1);
pub static CLONE_SNAPSHOT: c_int = (1 << 2);
pub static CLONE_KEEPBDEVTYPE: c_int = (1 << 3);
pub static CLONE_MAYBE_SNAPSHOT: c_int = (1 << 4);
pub static CLONE_MAXFLAGS: c_int = (1 << 5);
pub static CREATE_QUIET: c_int = (1 << 0);
pub static CREATE_MAXFLAGS: c_int = (1 << 1);

#[repr(C)]
pub struct snapshot {
    name: *mut c_char,
    comment_pathname: *mut c_char,
    timestamp: *mut c_char,
    lxcpath: *mut c_char,
    free: extern fn(c: *mut container)
}

#[repr(C)]
pub struct _zfs {
    zfsroot: *mut c_char
}

#[repr(C)]
pub struct _lvm {
    vg: *mut c_char,
    lv: *mut c_char,
    thinpool: *mut c_char
}

#[repr(C)]
pub struct _rbd {
    rbdname: *mut c_char,
    rbdpool: *mut c_char
}

#[repr(C)]
pub struct bdev_specs {
    pub fstype: *mut c_char,
    pub fssize: c_ulonglong,
    pub zfs: _zfs,
    pub lvm: _lvm,
    pub rbd: _rbd
}

pub enum migrate {
    PRE_DUMP,
    DUMP,
    RESTORE
}

#[repr(C)]
pub struct migrate_opts {
    directory: *mut c_char,
    verbose: c_bool,
    stop: c_bool,
    predump_dir: *mut c_char,
    pageserver_address: *mut c_char,
    pageserver_port: *mut c_char,
    preserve_inodes: c_bool
}

pub enum lock {}
pub enum conf {}

#[repr(C)]
pub struct container {
    pub name: *mut c_char,
    pub configfile: *mut c_char,
    pub pidfile: *mut c_char,
    pub slock: *mut lock,
    pub privlock: *mut lock,
    pub numthreads: c_int,
    pub lxc_conf: *mut conf,
    pub error_string: *mut c_char,
    pub error_num: c_int,
    pub daemonize: c_int,
    pub config_path: *mut c_char,
    pub is_defined: extern fn(c: *mut container) -> c_bool,
    pub state: extern fn(c: *mut container) -> *const c_char,
    pub is_running: extern fn(c: *mut container) -> c_bool,
    pub freeze: extern fn(c: *mut container) -> c_bool,
    pub unfreeze: extern fn(c: *mut container) -> c_bool,
    // TODO: Not sure that c_uint is the representation of pid_t
    pub init_pid: extern fn(c: *mut container) -> c_uint,
    pub load_config: extern fn(c: *mut container, alt_file: *const c_char) -> c_bool,
    pub start: extern fn(c: *mut container, useinit: c_int, argv: *const *mut c_char) -> c_bool,
    pub startl: extern fn(c: *mut container, useinit: c_int, ...) -> c_bool,
    pub stop: extern fn(c: *mut container) -> c_bool,
    pub want_daemonize: extern fn(c: *mut container, state: c_bool) -> c_bool,
    pub want_close_all_fds: extern fn(c: *mut container, state: c_bool) -> c_bool,
    pub config_file_name: extern fn(c: *mut container) -> *mut c_char,
    pub wait: extern fn(c: *mut container, state: c_bool, timeout: c_int) -> c_bool,
    pub set_config_item: extern fn(c: *mut container, key: *const c_char, value: *const c_char) -> c_bool,
    pub destroy: extern fn(c: *mut container) -> c_bool,
    pub save_config: extern fn(c: *mut container, alt_file: *const c_char) -> c_bool,
    pub create: extern fn(c: *mut container, t: *const c_char, bdevtype: *const c_char, specs: *mut bdev_specs, flags: c_int, argv: *const *mut c_char) -> c_bool,
    pub createl: extern fn(c: *mut container, t: *const c_char, bdevtype: *const c_char, specs: *mut bdev_specs, flags: c_int, ...) -> c_bool,
    pub rename: extern fn(c: *mut container, newname: *const c_char) -> c_bool,
    pub reboot: extern fn(c: *mut container) -> c_bool,
    pub shutdown: extern fn(c: *mut container, timeout: c_int) -> c_bool,
    pub clear_config: extern fn(c: *mut container) -> c_bool,
    pub clear_config_item: extern fn(c: *mut container, key: *const c_char) -> c_bool,
    pub get_config_item: extern fn(c: *mut container, key: *const c_char, retv: *mut c_char, inlen: c_int) -> c_bool,
    pub get_running_config_item: extern fn(c: *mut container, key: *const c_char) -> c_bool,
    pub get_keys: extern fn(c: *mut container, key: *const c_char, retv: *mut c_char, inlen: c_int) -> c_bool,
    pub get_interfaces: extern fn(c: *mut container) -> *mut *mut c_char,
    pub get_ips: extern fn(c: *mut container, interface: *const c_char, family: *const c_char, scope: c_int) -> *mut *mut c_char,
    pub get_cgroup_item: extern fn(c: *mut container, subsys: *const c_char, char: *mut c_char, inlen: c_int) -> c_bool,
    pub set_cgroup_item: extern fn(c: *mut container, subsys: *const c_char, value: *mut c_char) -> c_bool,
    pub get_config_path: extern fn(c: *mut container) -> c_bool,
    pub set_config_path: extern fn(c: *mut container, path: *const c_char) -> c_bool,
    pub clone: extern fn(c: *mut container, newname: *const c_char, lxcpath: *const c_char, flags: c_int, bdevtype: *const c_char, bdevdata: *const c_char, newsizei: c_ulonglong, hookargs: *mut *mut c_char) -> container,
    pub console_getfd: extern fn(c: *mut container, ttynum: *mut c_int, masterfd: *mut c_int) -> c_int,
    pub console: extern fn(c: *mut container, ttynum: c_int, stdinfd: c_int, stdoutfd: c_int, stderrfd: c_int, escape: c_int) -> c_int,
    pub attach: extern fn(c: *mut container, exec_function: extern fn(payload: *mut c_void) -> c_int, exec_payload: *mut c_void, options: *mut attach_options, attached_process: c_uint) -> c_int,
    pub attach_run_wait: extern fn(c: *mut container, options: *mut attach_options, program: *const c_char, argv: *const *mut c_char) -> c_int,
    pub attach_run_waitl: extern fn(c: *mut container, options: *mut attach_options, program: *const c_char, arg: *const c_char, ...) -> c_int,
    pub snapshot: extern fn(c: *mut container, commentfile: *const c_char) -> c_int,
    pub snapshot_list: extern fn(c: *mut container, snapshots: *mut *mut snapshot) -> c_int,
    pub snapshot_restore: extern fn(c: *mut container, snapname: *const c_char, newname: *const c_char) -> c_bool,
    pub snapshot_destroy: extern fn(c: *mut container, snapname: *const c_char) -> c_bool,
    pub may_control: extern fn(c: *mut container) -> c_bool,
    pub add_device_node: extern fn(c: *mut container, src_path: *const c_char, dest_path: *const c_char) -> c_bool,
    pub remove_device_node: extern fn(c: *mut container, src_path: *const c_char, dest_path: *const c_char) -> c_bool,
    pub attach_interface:  extern fn(c: *mut container, dev: *const c_char, dst_dev: *const c_char) -> c_bool,
    pub detach_interface:  extern fn(c: *mut container, dev: *const c_char, dst_dev: *const c_char) -> c_bool,
    pub checkpoint: extern fn(c: *mut container, directory: *mut c_char, stop: c_bool, verbose: c_bool) -> c_bool,
    pub restore: extern fn(c: *mut container, directory: *mut c_char, verbose: c_bool) -> c_bool,
    pub destroy_with_snapshots: extern fn(c: *mut container) -> c_bool,
    pub snapshot_destroy_all: extern fn(c: *mut container) -> c_bool,
    pub migrate: extern fn(c: *mut container, cmd: c_uint, opts: *mut migrate_opts, size: c_uint) -> c_int
}

#[link(name = "lxc")]
extern "C" {
    pub fn lxc_attach_run_command(payload: *mut c_void) -> c_int;
    pub fn lxc_attach_run_shell(payload: *mut c_void) -> c_int;
    pub fn lxc_container_new(name: *const c_char, configpath: *const c_char) -> *mut container;
    pub fn lxc_container_get(c: *mut container) -> c_int;
    pub fn lxc_container_put(c: *mut container) -> c_int;
    pub fn lxc_get_wait_states(states: *const *mut c_char) -> c_int;
    pub fn lxc_get_global_config_item(key: *const c_char) -> *const c_char;
    pub fn lxc_get_version() -> *const c_char;
    pub fn list_defined_containers(lxcpath: *const c_char, names: *mut *mut *mut c_char, cret: *mut *mut *mut container) -> c_int;
    pub fn list_active_containers(lxcpath: *const c_char, names: *mut *mut *mut c_char, cret: *mut *mut *mut container) -> c_int;
    pub fn list_all_containers(lxcpath: *const c_char, names: *mut *mut *mut c_char, cret: *mut *mut *mut container) -> c_int;
    pub fn lxc_log_close();
}
