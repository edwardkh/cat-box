from persistent_state.state import get_state_of

try:
    import alert.secrets as secrets
except ImportError:
    import alert.default_secrets as secrets

bin_full_cycles = 20


def get_bin_full_cycles():
    return get_state_of("bin_full_cycles", bin_full_cycles)


def get_smtp_server():
    return get_state_of("smtp_server", secrets.smtp_server)


def get_smtp_port():
    return get_state_of("smtp_port", secrets.smtp_port)


def get_send_email():
    return get_state_of("send_email", secrets.send_email)


def get_send_password():
    return get_state_of("send_password", secrets.send_password)


def get_bin_full_recipients():
    return get_state_of("bin_full_recipients", secrets.bin_full_recipients)
