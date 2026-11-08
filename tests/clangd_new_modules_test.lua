-- Run: nvim --headless -u NONE -i NONE -l tests/clangd_new_modules_test.lua
local root = vim.fs.normalize(vim.fn.getcwd() .. "/build/clangd-regression/nvim-project")
vim.fn.mkdir(root .. "/tools", "p")
vim.fn.mkdir(root .. "/src", "p")
vim.fn.mkdir(root .. "/build/gcc16/debug", "p")
vim.fn.writefile({ "{}" }, root .. "/CMakePresets.json")
vim.fn.writefile({ "-- fixture" }, root .. "/tools/clangd-new-modules.lua")
local database = root .. "/build/gcc16/debug/compile_commands.json"
vim.fn.writefile({ "[]" }, database)
local buffer = vim.api.nvim_create_buf(true, false)
local path = root .. "/src/new-component.cppm"
vim.api.nvim_buf_set_name(buffer, path)
local calls, stops, starts = 0, 0, 0
local original_system, original_clients, original_start = vim.system, vim.lsp.get_clients, vim.lsp.start
local complete
vim.system = function(command, options, callback)
  assert(command[1] == "cmake" and command[2] == "--preset" and command[3] == "gcc16-debug")
  assert(vim.fs.normalize(options.cwd) == root)
  calls = calls + 1
  complete = callback
end
vim.lsp.get_clients = function()
  return {{ config = { name = "clangd", root_dir = root }, attached_buffers = { [buffer] = true },
    stop = function() stops = stops + 1 end }}
end
vim.lsp.start = function(_, options)
  assert(options.bufnr == buffer)
  starts = starts + 1
end
local helper = dofile("tools/clangd-new-modules.lua")
helper.attach(buffer)
helper.attach(buffer) -- No duplicate autocmds on repeated attachments.
vim.api.nvim_exec_autocmds("BufWritePost", { buffer = buffer })
assert(calls == 1, "New modules must trigger CMake")
complete { code = 0, stdout = "", stderr = "" }
assert(vim.wait(1500, function() return starts == 1 end), "clangd must restart after database refresh")
assert(stops == 1)
vim.fn.writefile({ vim.json.encode({ { file = path } }) }, database)
vim.api.nvim_exec_autocmds("BufWritePost", { buffer = buffer })
assert(calls == 1, "Existing modules must not repeatedly configure CMake")
print("New module refresh, scoped clangd restart, duplicate attachment, and existing-file checks passed")
vim.system, vim.lsp.get_clients, vim.lsp.start = original_system, original_clients, original_start
vim.cmd("qa!")
