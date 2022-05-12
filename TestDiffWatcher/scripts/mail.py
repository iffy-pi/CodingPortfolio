import os
#mail libraries
import smtplib
import datetime
from email.mime.base import MIMEBase
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.utils import COMMASPACE
from email import encoders

'''
GENERAL FILE THAT CONTAINS ALL FUNCTIONS USED FOR CONFLUENCE RELATED ACTIONS
USED BY POST_TO_CONFLUENCE(AND LUXSDK), TEST_DIFF and POST_TEST_EMAIL LUXSDK
'''
def send_text(text, subject, sender, mainRecipient, recipients, files=[], important=False, server="torsmtp.amd.com", content="text", verbose=False):
    #print('In emailing')
    if not isinstance(recipients, list):
        raise Exception("{0} error: {1}".format(__file__, "recipients must be a list"))
    
    msg = MIMEMultipart()
    msg['Subject'] = subject
    msg['From'] = sender
    msg['To'] = mainRecipient
    msg['Cc'] = ( ";".join(recipients) ).replace("\n\r ", "")
    if important:
        msg['X-Priority'] = "1"
        msg['X-MSMail-Priority'] = "High"

    if content == "text":
        msg.attach( MIMEText(text) )
    else:
        msg.attach( MIMEText(text, content) )
    
    for filename in files:
        if verbose:
            print("Attaching file ({0})".format(filename))
        with open(filename, "rb") as file:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload( file.read() )
            encoders.encode_base64(part)
            part.add_header('Content-Disposition', 'attachment; filename="{0}"'.format(os.path.basename(filename)))
            msg.attach(part)

    smtp = smtplib.SMTP(server)
    #print(smtp.noop())
    smtp.sendmail(sender, recipients, msg.as_string())

    if verbose:
        print(msg.as_string())
    smtp.quit()
    return 1

def send_email_from_farm(text, subject, main_recipient, recipients, files=[], content="text"):
    #sends an email with the given content from the maximator farm account
    if main_recipient not in recipients:
        recipients.append(main_recipient)

    send_text(text, subject, 'Maximator.Farm@amd.com', main_recipient, recipients, files=files, content=content)
    print('SENT')