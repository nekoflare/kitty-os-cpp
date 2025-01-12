import curses
import subprocess
import re

def run_script(stdscr):
    stdscr.clear()
    height, width = stdscr.getmaxyx()
    left_width = width // 2
    right_width = width - left_width

    left_win = curses.newwin(height, left_width, 0, 0)
    right_win = curses.newwin(height, right_width, 0, left_width)

    undefined_ref_pattern = re.compile(r'(.*):(\d+): undefined reference to [`"]?(.*?)["`]?$')
    process = subprocess.Popen(["bash", "./all.sh"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

    left_buffer = []
    right_buffer = []

    while True:
        line = process.stdout.readline()
        if line:
            left_buffer.append(line.strip())
            match = undefined_ref_pattern.search(line)
            if match:
                right_buffer.append(line.strip())

            if len(left_buffer) > height:
                left_buffer.pop(0)
            if len(right_buffer) > height:
                right_buffer.pop(0)

            left_win.clear()
            for i, l in enumerate(left_buffer[-height:]):
                left_win.addstr(i, 0, l[:left_width - 1])
            left_win.refresh()

            right_win.clear()
            for i, r in enumerate(right_buffer[-height:]):
                right_win.addstr(i, 0, r[:right_width - 1])
            right_win.refresh()

        if process.poll() is not None and not line:
            break

    process.wait()

    # Wait for user input before exiting
    stdscr.nodelay(False)
    stdscr.addstr(height - 1, 0, "Press any key to exit...")
    stdscr.refresh()
    stdscr.getch()

    # Finalize curses properly
    curses.endwin()

    # Keep the output visible after exiting
    print("\n".join(left_buffer))
    print("\nUndefined References:")
    print("\n".join(right_buffer))

if __name__ == "__main__":
    curses.wrapper(run_script)

