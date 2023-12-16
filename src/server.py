from alert.settings import get_bin_full_cycles
from alert.bin_full import get_current_cycles
from litter_box import state
from litter_box.settings import get_load_sensor_threshold, get_timed_cycle_delay_hours
from litter_box.state import EMPTYING, PAUSED
from microdot_asyncio import Microdot
from wifi.connection import get_base_url
from persistent_state.state import merge_state

app = Microdot()

htmldoc = '''
<!DOCTYPE html>
<html lang="en-US">
<head>
    <title>Cat Box</title>
    <meta charset="UTF-8">
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <style>
        button {
            width: 100%;
            padding: 5px;
            margin: 5px 0px;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>Cat Box</h1>
    <ul class="collapsible">
        <li class="active">
            <div class="collapsible-header"><i class="material-icons">input</i>Actions</div>
            <div class="collapsible-body">
                <div class="row">
                    <div class="center-align col s12 m3">
                        <button id="resetButton" class="waves-effect waves-light btn blue darken-4"
                                onclick="reset();"><i class="material-icons">add_box</i>Reset
                        </button>
                    </div>
                    <div class="center-align col s12 m3">
                        <button id="cycleButton" class="waves-effect waves-light btn green" onclick="cycle();"><i
                                class="material-icons">loop</i>Cycle
                        </button>
                    </div>
                    <div class="center-align col s12 m3">
                        <button id="emptyButton" class="waves-effect waves-light btn red darken-3" onclick="empty();">
                            <i class="material-icons">open_in_new</i>Empty
                        </button>
                    </div>
                    <div id="pauseButtonBlock" class="center-align col s12 m3">
                        <button id="pauseButton" class="waves-effect waves-light btn blue lighten-2"
                                onclick="pause();"><i class="material-icons">pause_circle_outline</i>Pause
                        </button>
                    </div>
                    <div id="unpauseButtonBlock" hidden class="center-align col s12 m3">
                        <button id="unpauseButton" class="waves-effect waves-light btn orange" onclick="unpause();">
                            <i class="material-icons">play_circle_outline</i>Unpause
                        </button>
                    </div>
                </div>
            </div>
        </li>
        <li>
            <div class="collapsible-header"><i class="material-icons">settings</i>Settings</div>
            <div class="collapsible-body">
                <div>
                    <label for="sensitivity"><i class="material-icons">fitness_center</i>Sensitivity:</label>
                    <output id="sensitivityValue">''' + str(get_load_sensor_threshold()) + '''</output>
                    <input
                        id="sensitivity" type="range" min="0" max="6000"
                        value="''' + str(get_load_sensor_threshold()) + '''"
                        onchange="setSensitivity(this.value)"/>
                </div>
                <div>
                    <label for="alertCycles"><i class="material-icons">alarm_add</i>Current Cycle Count:</label>
                    <span id="currentCycles">''' + str(get_current_cycles()) + '''</span>
                    <label for="alertCycles"><i class="material-icons">alarm</i>Alert Cycles:</label>
                    <output id="alertCyclesValue">''' + str(get_bin_full_cycles()) + '''</output>
                    <input
                        id="alertCycles" type="range" min="0" max="30"
                        value="''' + str(get_bin_full_cycles()) + '''"
                        onchange="setBinFullCycles(this.value)"/>
                </div>
                <div>
                    <label for="timedCycleDelayHours"><i class="material-icons">timelapse</i>Timed Cycle Delay (Hours):</label>
                    <output id="timedCycleDelayHoursValue">''' + str(get_timed_cycle_delay_hours()) + '''</output>
                    <input
                        id="timedCycleDelayHours" type="range" min="4" max="24" step="1"
                        value="''' + str(get_timed_cycle_delay_hours()) + '''"
                        onchange="setTimedCycleDelayHours(this.value)"/>
                </div>
            </div>
        </li>
    </ul>
    <div>
        Current State: <span id='currentState'></span>
    </div>

    <script src="https://unpkg.com/axios/dist/axios.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js"></script>
    <script>
        const handleStateChange = (state) => {
            document.getElementById("currentState").innerHTML = state;
            let pauseButton = document.getElementById("pauseButtonBlock");
            let unpauseButton = document.getElementById("unpauseButtonBlock");
            if(state == "''' + PAUSED + '''"){
                pauseButton.setAttribute("hidden", "hidden");
                unpauseButton.removeAttribute("hidden");
            }else{
                unpauseButton.setAttribute("hidden", "hidden");
                pauseButton.removeAttribute("hidden");
            }
        };

        const updateSatus = () => {
            axios.get("/current-state")
                .then((response) => {
                    handleStateChange(response.data.state)
                })
                .catch((error) => console.error(error));
        };

        const action = (action) => {
            axios.post("/"+action)
                .then((response) => {
                    handleStateChange(response.data.state)
                })
                .catch((error) => console.error(error));
        }
        
        const update = (settings) => {
            axios.patch("/settings", settings)
                .then((response) => {
                    M.toast({html: 'Setting updated!'})
                })
                .catch((error) => console.error(error));
        }
        
        updateSatus()
        setInterval(updateSatus, 10000)

        const cycle = () => {
            action("cycle")
        }

        const empty = () => {
            action("empty")
        }

        const pause = () => {
            action("pause")
        }

        const unpause = () => {
            action("unpause")
        }

        const reset = () => {
            action("reset")
        }

        const setSensitivity = (value) => {
            document.querySelector("#sensitivityValue").textContent = value;
            update({"load_sensor_threshold": parseInt(value)})
        }
        
        const setBinFullCycles = (value) => {
            document.querySelector("#alertCyclesValue").textContent = value;
            update({"bin_full_cycles": parseInt(value)})
        }

        document.addEventListener('DOMContentLoaded', function() {
            var elements = document.querySelectorAll('.collapsible');
            var instances = M.Collapsible.init(elements);
        });

    </script>
</div>
</body>
</html>
'''


@app.route('/')
def hello(request):
    return htmldoc, 200, {'Content-Type': 'text/html'}


@app.route('/current-state')
def get_state(request):
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/cycle', methods=['POST'])
def set_state(request):
    state.start_cycle()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/empty', methods=['POST'])
def set_state(request):
    state.set_state(EMPTYING)
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/pause', methods=['POST'])
def pause(request):
    print("Request to pause")
    state.pause()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/unpause', methods=['POST'])
def pause(request):
    print("Request to un-pause")
    state.unpause()
    return {"state": state.get_state()}, 200, {'Content-Type': 'application/json'}


@app.route('/reset', methods=['GET'])
def reset(request):
    print("Request to reset")
    state.reset()
    return {"state": "Reset"}, 302, {'Content-Type': 'application/json', 'Location': get_base_url()}


@app.route('/reset', methods=['POST'])
def reset(request):
    print("Request to reset")
    state.reset()
    return {"state": "Reset"}, 200, {'Content-Type': 'application/json'}


@app.route('/settings', methods=['PATCH'])
def reset(request):
    print("Request to patch settings")
    print(request.json)
    merge_state(request.json)
    return {"state": "Updated"}, 200, {'Content-Type': 'application/json'}


@app.route('/settings', methods=['GET'])
def reset():
    settings = {
        "load_sensor_threshold": get_load_sensor_threshold()
    }
    return settings, 200, {'Content-Type': 'application/json'}


async def start():
    await app.start_server(debug=True, port=80)
