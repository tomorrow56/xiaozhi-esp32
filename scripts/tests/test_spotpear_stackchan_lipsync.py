"""Exercise the board-local playback envelope with synthetic PCM, no ESP hardware."""
import pathlib
import shutil
import subprocess
import tempfile
import unittest


class SpotPearStackChanLipSyncTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('c++'), 'host C++ compiler required')
    def test_pcm_silence_volume_expiry_reset_and_tick_wrap(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        include = root / 'main/boards/spotpear/spotpear-esp32-ai-1.28'
        driver = r'''
#include "stackchan_lipsync.h"
#include <cassert>
#include <cmath>
#include <limits>
int main() {
    StackChanLipSync state;
    int16_t silence[480]{};
    int16_t voice[480];
    for (int i = 0; i < 480; ++i) voice[i] = (i & 1) ? 12000 : -12000;
    assert(state.Level(0) == 0);
    state.Publish(silence, 480, 70, 100);
    assert(state.Level(110) == 0);
    state.Publish(voice, 480, 70, 200, 80);
    const float speaking = state.Level(220);
    assert(speaking > 0.5f && speaking <= 1);
    state.Publish(voice, 480, 10, 300);
    assert(state.Level(320) < speaking);
    state.Publish(voice, 480, 0, 400);
    assert(state.Level(420) == 0);
    state.Publish(voice, 480, 70, 500, 80);
    assert(state.Level(580) > 0);
    assert(state.Level(581) == 0); // No stale talking mouth after stop/underrun.
    state.Publish(voice, 480, 70, 600);
    state.Reset();
    assert(state.Level(601) == 0); // Output disabled / interrupted.
    state.Publish(nullptr, 480, 70, 700);
    assert(state.Level(701) == 0);
    state.Publish(voice, 0, 70, 800);
    assert(state.Level(801) == 0);
    int16_t negative[] = {std::numeric_limits<int16_t>::min()};
    state.Publish(negative, 1, 100, 900);
    assert(std::isfinite(state.Level(901)) && state.Level(901) <= 1);
    state.Publish(voice, 480, 70, UINT32_MAX - 20, 80);
    assert(state.Level(10) > 0);
    assert(state.Level(100) == 0);
}
'''
        with tempfile.TemporaryDirectory() as temp:
            binary = pathlib.Path(temp) / 'lipsync-test'
            subprocess.run(['c++', '-std=c++17', '-fsanitize=undefined',
                            '-I' + str(include), '-x', 'c++', '-', '-o', str(binary)],
                           input=driver, text=True, check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
