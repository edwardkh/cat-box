import alert.umail as umail
import alert.secrets as secrets
import re


def send(emails, subject, message):
    for email in emails:
        smtp = umail.SMTP(secrets.smtp_server, secrets.smtp_port, username=secrets.send_email,
                          password=secrets.send_password)
        smtp.to(email)
        smtp.write("From: Cat Box <" + secrets.send_email + ">\n")
        smtp.write("To: " + email + "\n")
        if re.match("^\d{10}@.*", email):
            smtp.write("Subject: " + subject + "\n")
        smtp.write("\n")
        smtp.write(message)
        smtp.send()
        smtp.quit()
