import alert.umail as umail
from alert.settings import get_smtp_email, get_smtp_port, get_smtp_password, get_smtp_server
import re


def send(emails, subject, message):
    for email in emails:
        smtp = umail.SMTP(get_smtp_server(), get_smtp_port(), username=get_smtp_email(), password=get_smtp_password())
        smtp.to(email)
        smtp.write("From: Cat Box <" + get_smtp_email() + ">\n")
        smtp.write("To: " + email + "\n")
        if re.match("^\d{10}@.*", email):
            smtp.write("Subject: " + subject + "\n")
        smtp.write("\n")
        smtp.write(message)
        smtp.send()
        smtp.quit()
