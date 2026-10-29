from alert.settings import get_bin_full_cycles
from alert.bin_full import get_current_cycles
from litter_box import state
from litter_box.settings import get_load_sensor_threshold, get_timed_cycle_delay_hours
from litter_box.state import EMPTYING, PAUSED
from microdot_asyncio import Microdot
from wifi.connection import get_base_url
from persistent_state.state import merge_state

app = Microdot()


@app.route('/')
def hello(request):
    file = open("frontend/main.html", "r")
    return file.read(), 200, {'Content-Type': 'text/html'}


@app.route('/main.js')
def script(request):
    file = open("frontend/main.js", "r")
    return file.read(), 200, {'Content-Type': 'text/javascript'}


@app.route('/current-state')
def get_state(request):
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/cycle', methods=['POST'])
def cycle(request):
    state.start_cycle()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/empty', methods=['POST'])
def empty(request):
    state.set_state(EMPTYING)
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/pause', methods=['POST'])
def pause(request):
    print("Request to pause")
    state.pause()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/unpause', methods=['POST'])
def unpause(request):
    print("Request to un-pause")
    state.unpause()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/reset', methods=['GET'])
def get_reset(request):
    print("Request to reset")
    state.reset()
    return {"state": "Reset"}, 302, {'Content-Type': 'application/json', 'Location': get_base_url()}


@app.route('/reset', methods=['POST'])
def post_reset(request):
    print("Request to reset")
    state.reset()
    return {"state": "Reset"}, 200, {'Content-Type': 'application/json'}


@app.route('/settings', methods=['PATCH'])
def patch_settings(request):
    print("Request to patch settings")
    print(request.json)
    merge_state(request.json)
    return {"state": "Updated"}, 200, {'Content-Type': 'application/json'}


@app.route('/settings', methods=['GET'])
def get_settings(request):
    settings = {
        "load_sensor_threshold": get_load_sensor_threshold(),
        "timed_cycle_delay_hours": get_timed_cycle_delay_hours(),
        "bin_full_cycles": get_bin_full_cycles(),
        "current_cycles": get_current_cycles(),
        "last_cycle": state.get_last_cycle(),
        "cycle_wait_time": state.get_cycle_wait_time()
    }
    return settings, 200, {'Content-Type': 'application/json'}


async def start():
    await app.start_server(debug=True, port=80)
