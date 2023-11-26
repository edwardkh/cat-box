from litter_box import state
from litter_box.state import EMPTYING, PAUSED
from microdot_asyncio import Microdot
from wifi.connection import get_base_url
from persistent_state.state import merge_state

app = Microdot()

htmldoc = '''<!DOCTYPE html>
<html>
    <head>
        <title>Cat Box</title>        <meta charset="UTF-8">
        <script src="https://unpkg.com/axios/dist/axios.min.js"></script>
        <script>
            const handleStateChange = (state) => {
                document.getElementById("currentState").innerHTML = state;
                let pauseButton = document.getElementById("pauseButton");
                let unpauseButton = document.getElementById("unpauseButton");
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
        </script>
    </head>
    <body>
        <div>
            <h1>Cat Box</h1>
            <input id="cycleButton" type="button" value="Cycle" onclick="cycle();" />
            <input id="emptyButton" type="button" value="Empty" onclick="empty();" />
            <input id="pauseButton" type="button" value="Pause" onclick="pause();" />
            <input id="unpauseButton" hidden type="button" value="Unpause" onclick="unpause();" />
            <input id="resetButton" type="button" value="Reset" onclick="reset();" />
            <div>
                Current State: <span id='currentState'></span>
            </div>
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


async def start():
    await app.start_server(debug=True, port=80)
