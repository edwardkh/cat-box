import ujson

cached_state = None


def load_state():
    global cached_state
    try:
        f = open('/persistent_state/state.json')
        json_string = f.read()
        f.close()
    except IOError:
        json_string = "{}"
    if not json_string:
        json_string = "{}"
    cached_state = ujson.loads(json_string)


def get_state():
    global cached_state
    if not cached_state:
        load_state()
    return cached_state


def set_state(state):
    global cached_state
    cached_state = state
    f = open('/persistent_state/state.json', 'w')
    f.write(ujson.dumps(state))
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
