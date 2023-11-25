import ujson


def get_state_of(key):
    f = open('/persistent_state/state.json')
    saved_state = ujson.loads(f.read())
    f.close()
    value = saved_state.get(key, None)
    return value


def set_state_of(key, value):
    f = open('/persistent_state/state.json')
    saved_state = ujson.loads(f.read())
    saved_state[key] = value
    f.close()
    f = open('/persistent_state/state.json', 'w')
    f.write(ujson.dumps(saved_state))
    f.close()


def increment_state_of(key):
    f = open('/persistent_state/state.json')
    json_string = f.read()
    print(f"persisted state: {json_string}")
    saved_state = ujson.loads(json_string)
    saved_state[key] = saved_state.get(key, 0) + 1
    f.close()
    f = open('/persistent_state/state.json', 'w')
    f.write(ujson.dumps(saved_state))
    f.close()
    return saved_state[key]
