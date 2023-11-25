from persistent_state.state import get_state_of

try:
    import wifi.secrets as secrets
except ImportError:
    import wifi.default_secrets as secrets

connection_retries = 3
connection_backoff = 60


def get_connection_retries():
    return get_state_of("wifi_connection_retries", connection_retries)


def get_connection_backoff():
    return get_state_of("wifi_connection_backoff", connection_backoff)


def get_wifi_ssid():
    return get_state_of("wifi_ssid", secrets.wifi_ssid)


def get_wifi_password():
    return get_state_of("wifi_password", secrets.wifi_password)


def get_wifi_ip_config():
    return get_state_of("wifi_ip_config", secrets.wifi_ip_config)


def get_base_url():
    return get_state_of("base_url", secrets.base_url)
