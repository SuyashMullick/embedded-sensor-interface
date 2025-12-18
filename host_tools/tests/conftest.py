import pytest

def pytest_addoption(parser):
    parser.addoption("--pty", action="store", help="Path to Zephyr native_sim PTY link (e.g. ./pty.link)")
