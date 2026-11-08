"""Exercise real clangd property completion without creating source files.

Run from the repository root: python tests/clay_completion_test.py
Requires clangd on PATH and the project's existing compilation database.
"""

import json
import os
from pathlib import Path
import queue
import shutil
import subprocess
import threading
import time


ROOT = Path(__file__).resolve().parent.parent


def main():
    executable = shutil.which(os.environ.get("CLANGD", "clangd"))
    if not executable:
        raise RuntimeError("clangd must be available on PATH (or set CLANGD)")
    process = subprocess.Popen(
        [executable, "--experimental-modules-support", "--log=error"],
        cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
    )
    messages = queue.Queue()

    def read_messages():
        while True:
            headers = {}
            while True:
                line = process.stdout.readline()
                if not line:
                    messages.put({"closed": True})
                    return
                if line in (b"\r\n", b"\n"):
                    break
                key, value = line.decode().split(":", 1)
                headers[key.lower()] = value.strip()
            messages.put(json.loads(process.stdout.read(int(headers["content-length"]))))

    threading.Thread(target=read_messages, daemon=True).start()

    def send(message):
        body = json.dumps({"jsonrpc": "2.0", **message}).encode()
        process.stdin.write(f"Content-Length: {len(body)}\r\n\r\n".encode() + body)
        process.stdin.flush()

    def wait_for(predicate):
        deadline = time.monotonic() + 45
        while True:
            message = messages.get(timeout=max(0.1, deadline - time.monotonic()))
            if message.get("closed"):
                raise RuntimeError("clangd exited before responding")
            if predicate(message):
                return message
            if "id" in message and "method" in message:
                send({"id": message["id"], "result": None})
            if time.monotonic() >= deadline:
                raise TimeoutError("Timed out waiting for clangd")

    def request(identifier, method, params):
        send({"id": identifier, "method": method, "params": params})
        message = wait_for(lambda item: item.get("id") == identifier and
                           ("result" in item or "error" in item))
        if "error" in message:
            raise RuntimeError(message["error"])
        return message.get("result")

    try:
        request(1, "initialize", {
            "processId": os.getpid(), "rootUri": ROOT.as_uri(),
            "capabilities": {"textDocument": {"completion": {"completionItem": {"snippetSupport": True}}}},
        })
        send({"method": "initialized", "params": {}})
        cases = [
            ("element", "CLAY({ .CURSOR }) {}", {"id", "layout", "backgroundColor"}),
            ("layout", "CLAY({ .layout = { .CURSOR } }) {}", {"sizing", "padding", "childGap", "layoutDirection"}),
            ("sizing", "CLAY({ .layout = { .sizing = { .CURSOR } } }) {}", {"width", "height"}),
            ("alignment", "CLAY({ .layout = { .childAlignment = { .CURSOR } } }) {}", {"x", "y"}),
            ("border", "CLAY({ .border = { .CURSOR } }) {}", {"color", "width"}),
            ("text", 'CLAY_TEXT(CLAY_STRING("test"), CLAY_TEXT_CONFIG({ .CURSOR }));', {"fontSize", "textColor"}),
        ]
        for identifier, (name, body, expected) in enumerate(cases, 2):
            source = "#include <clay.h>\nvoid render() { " + body + " }\n"
            offset = source.index("CURSOR")
            source = source.replace("CURSOR", "")
            uri = (ROOT / "build" / "completion-check" / (name + ".cpp")).as_uri()
            send({"method": "textDocument/didOpen", "params": {"textDocument": {
                "uri": uri, "languageId": "cpp", "version": 1, "text": source,
            }}})
            # Cold-start completions can be lexical fallbacks. Wait for the AST.
            wait_for(lambda item: item.get("method") == "textDocument/publishDiagnostics" and
                     item.get("params", {}).get("uri") == uri)
            result = request(identifier, "textDocument/completion", {
                "textDocument": {"uri": uri}, "position": {
                    "line": source[:offset].count("\n"),
                    "character": len(source[:offset].rsplit("\n", 1)[-1]),
                }, "context": {"triggerKind": 1},
            })
            items = result.get("items", []) if isinstance(result, dict) else result or []
            labels = {item["label"].strip() for item in items}
            assert expected <= labels, f"{name}: missing {expected - labels}; received {labels}"
            print(f"{name}: {len(labels)} properties, expected fields present", flush=True)
        for name, source in [
            ("named-and-inline", '#include <clay.h>\nvoid render() { '
             'Clay_ElementDeclaration config{}; CLAY(config) {} '
             'CLAY({ .layout = { .childGap = 12 } }) { CLAY({}) {} } '
             'Clay_TextElementConfig text{}; CLAY_TEXT_CONFIG(text); }\n'),
            ("stationary-module", (ROOT / "src/components/ui/stationary.cppm").read_text(encoding="utf-8")),
        ]:
            path = ROOT / "src/components/ui/stationary.cppm" if name == "stationary-module" else (
                ROOT / "build/completion-check/valid-config.cpp")
            uri = path.as_uri()
            send({"method": "textDocument/didOpen", "params": {"textDocument": {
                "uri": uri, "languageId": "cpp", "version": 1, "text": source,
            }}})
            notification = wait_for(lambda item: item.get("method") == "textDocument/publishDiagnostics" and
                                    item.get("params", {}).get("uri") == uri)
            errors = [item["message"] for item in notification["params"]["diagnostics"] if item.get("severity") == 1]
            assert not errors, f"{name}: {errors}"
            print(f"{name}: no source errors", flush=True)
        request(100, "shutdown", None)
        send({"method": "exit", "params": {}})
    finally:
        if process.poll() is None:
            process.terminate()
        process.wait(timeout=5)


if __name__ == "__main__":
    main()
