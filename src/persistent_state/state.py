import ujson

cached_state = None


def load_state():
    global cached_state
    state = {}
    try:
        f = open('/persistent_state/state.json')
        json_string = f.read()
        print("Loading: " + json_string)
        f.close()
        state = ujson.loads(json_string)
    except:
        pass
    cached_state = state


def get_state():
    global cached_state
    if not cached_state:
        load_state()
    return cached_state


def merge_state(state):
    merged_state = get_state()
    for (key, value) in state.items():
        merged_state[key] = value
    set_state(merged_state)


def set_state(state):
    global cached_state
    if not state:
        state = {}
    cached_state = state
    f = open('/persistent_state/state.json', 'w')
    json = ujson.dumps(state)
    print("Persisting state: " + json)
    f.write(json)
    f.close()


def get_state_of(key, default=None):
    return get_state().get(key, default)


def set_state_of(key, value):
    saved_state = get_state()
    saved_state[key] = value
    set_state(saved_state)


def increment_state_of(key):
    saved_state = get_state()
    saved_state[key] = saved_state.get(key, 0) + 1
    set_state(saved_state)
    return saved_state[key]
