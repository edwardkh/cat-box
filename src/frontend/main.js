const handleStateChange = (state) => {
    document.getElementById("currentState").innerHTML = state;
    let pauseButton = document.getElementById("pauseButtonBlock");
    let unpauseButton = document.getElementById("unpauseButtonBlock");
    if(state == "Paused"){
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

const getSettings = () => {
    axios.get("/settings")
        .then((response) => {
            console.log(response)
            document.getElementById("sensitivityValue").innerHTML=response.data.load_sensor_threshold
            document.getElementById("sensitivity").value=response.data.load_sensor_threshold

            document.getElementById("currentCycles").innerHTML=response.data.current_cycles

            document.getElementById("alertCyclesValue").innerHTML=response.data.bin_full_cycles
            document.getElementById("alertCycles").value=response.data.bin_full_cycles

            document.getElementById("timedCycleDelayHoursValue").innerHTML=response.data.timed_cycle_delay_hours
            document.getElementById("timedCycleDelayHours").value=response.data.timed_cycle_delay_hours
        })
        .catch((error) => console.error(error));
}

getSettings()
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