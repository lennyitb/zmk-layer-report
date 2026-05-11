#!/usr/bin/env ruby
# frozen_string_literal: true

require "fiddle"
require "fiddle/import"

USAGE_PAGE = 0xFF42
REPORT_SIZE = 4

MOD_NAMES = [
  "LCtrl", "LShift", "LAlt", "LGui",
  "RCtrl", "RShift", "RAlt", "RGui"
].freeze

# hid_device_info field offsets (ARM64 macOS, hidapi 0.13+)
OFF_PATH       = 0
OFF_VENDOR_ID  = 8
OFF_PRODUCT_ID = 10
OFF_USAGE_PAGE = 48
OFF_NEXT       = 56

module HidApi
  extend Fiddle::Importer

  lib_path = [
    "/opt/homebrew/lib/libhidapi.dylib",
    "/usr/local/lib/libhidapi.dylib",
    "libhidapi.dylib"
  ].find { |p| File.exist?(p) } || "libhidapi.dylib"

  dlload lib_path

  extern "int hid_init(void)"
  extern "int hid_exit(void)"
  extern "void* hid_enumerate(unsigned short, unsigned short)"
  extern "void hid_free_enumeration(void*)"
  extern "void* hid_open_path(const char*)"
  extern "int hid_read_timeout(void*, void*, int, int)"
  extern "void hid_close(void*)"
end

def read_ptr(base, offset)
  (base + offset)[0, 8].unpack1("Q")
end

def read_u16(base, offset)
  (base + offset)[0, 2].unpack1("v")
end

def read_cstr(ptr_addr)
  return nil if ptr_addr == 0
  Fiddle::Pointer.new(ptr_addr).to_s
end

def find_device
  devs = HidApi.hid_enumerate(0, 0)
  return nil if devs.null?

  result = nil
  cur = devs
  until cur.null?
    usage_page = read_u16(cur, OFF_USAGE_PAGE)
    if usage_page == USAGE_PAGE
      path = read_cstr(read_ptr(cur, OFF_PATH))
      vid  = read_u16(cur, OFF_VENDOR_ID)
      pid  = read_u16(cur, OFF_PRODUCT_ID)
      result = { path: path, vid: vid, pid: pid }
      break
    end
    next_addr = read_ptr(cur, OFF_NEXT)
    break if next_addr == 0
    cur = Fiddle::Pointer.new(next_addr)
  end

  HidApi.hid_free_enumeration(devs)
  result
end

def format_mods(modifiers, mod_flags)
  parts = []
  8.times do |i|
    next unless modifiers[i] == 1
    source = mod_flags[i] == 1 ? "sticky" : "held"
    parts << "#{MOD_NAMES[i]}(#{source})"
  end
  parts
end

def format_report(data)
  layer_state = data[0, 2].unpack1("v")
  modifiers   = data[2].ord
  mod_flags   = data[3].ord

  layers = (0...16).select { |i| layer_state[i] == 1 }
  ts = Time.now.strftime("%H:%M:%S")

  line = "[#{ts}] layers: #{layers.join(", ")}"

  mods = format_mods(modifiers, mod_flags)
  line += " | mods: #{mods.join(", ")}" unless mods.empty?

  line
end

# --- main ---

HidApi.hid_init

$stdout.puts "Layer Monitor — scanning for usage page 0x#{USAGE_PAGE.to_s(16).upcase}..."
$stdout.flush

dev_info = find_device
unless dev_info
  $stderr.puts "No device found with usage page 0x#{USAGE_PAGE.to_s(16).upcase}."
  $stderr.puts "Is the keyboard connected?"
  HidApi.hid_exit
  exit 1
end

$stdout.puts "Found: #{dev_info[:path]} (VID=%04x PID=%04x)" % [dev_info[:vid], dev_info[:pid]]

handle = HidApi.hid_open_path(dev_info[:path])
if handle.null?
  $stderr.puts "Failed to open device. On macOS, grant Input Monitoring permission"
  $stderr.puts "to your terminal app in System Settings > Privacy & Security."
  HidApi.hid_exit
  exit 1
end

$stdout.puts "Connected. Listening for state changes...\n\n"
$stdout.flush

buf = Fiddle::Pointer.malloc(REPORT_SIZE)
prev_data = nil
running = true

trap("INT") { running = false }

while running
  n = HidApi.hid_read_timeout(handle, buf, REPORT_SIZE, 1000)
  next if n == 0
  if n < 0
    $stderr.puts "Read error"
    break
  end

  data = buf[0, n]
  data = data.ljust(REPORT_SIZE, "\x00") if n < REPORT_SIZE

  next if data == prev_data
  prev_data = data

  $stdout.puts "  raw: #{data.bytes.map { |b| "%02x" % b }.join(" ")}"
  $stdout.puts format_report(data)
  $stdout.flush
end

$stdout.puts "\nDisconnected."
HidApi.hid_close(handle)
HidApi.hid_exit
