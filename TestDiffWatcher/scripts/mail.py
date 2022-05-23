import os
#mail libraries
import keyring
import smtplib
import datetime
from email.mime.base import MIMEBase
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.utils import COMMASPACE
from email import encoders

EMAIL_KEYRING = 'python_bot_gmail_credential'
EMAIL_USERNAME = 'iffysbot@gmail.com'

def send_email(text, subject, sender, mainRecipient, recipients, files=[], important=False, server="torsmtp.amd.com", content="text", verbose=False):
    if not isinstance(recipients, list):
        raise Exception("{0} error: {1}".format(__file__, "recipients must be a list"))
    
    if verbose: print('Configuring Email Headers')
    msg = MIMEMultipart()
    msg['Subject'] = subject
    msg['From'] = sender
    msg['To'] = mainRecipient
    msg['Cc'] = ( ";".join(recipients) ).replace("\n\r ", "")
    if important:
        msg['X-Priority'] = "1"
        msg['X-MSMail-Priority'] = "High"


    if verbose: print('Configuring Email Content')
    if content == "text":
        msg.attach( MIMEText(text) )
    else:
        msg.attach( MIMEText(text, content) )
    
    for filename in files:
        if verbose: print("Attaching file ({0})".format(filename))
        with open(filename, "rb") as file:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload( file.read() )
            encoders.encode_base64(part)
            part.add_header('Content-Disposition', 'attachment; filename="{0}"'.format(os.path.basename(filename)))
            msg.attach(part)

    if verbose: print(f'Connecting to SMTP server: {server}...')
    smtp = smtplib.SMTP(server)
    #print(smtp.noop())

    if verbose: print('Sending Email...')
    smtp.sendmail(sender, recipients, msg.as_string())
    if verbose: print('Sent')
    if verbose: print(msg.as_string())
    smtp.quit()
    return 1

def get_email_creds():
    pwd = keyring.get_password(EMAIL_KEYRING, EMAIL_USERNAME)

    if pwd is None:
        raise Exception(f'No {EMAIL_KEYRING} credential was found in keyring, please use public_testing.py to save credentials')

    return EMAIL_USERNAME, pwd

def set_email_creds(passwd):
    keyring.set_password(EMAIL_KEYRING, EMAIL_USERNAME, passwd)

    #verification
    stored_passwd =  keyring.get_password(EMAIL_KEYRING, EMAIL_USERNAME)

    if stored_passwd is None or stored_passwd != passwd:
        return 1
    
    return 0

def send_email_from_bot(text, subject, mainRecipient, recipients, files=[], important=False, content="text", verbose=False):
    if not isinstance(recipients, list):
        raise Exception("{0} error: {1}".format(__file__, "recipients must be a list"))

    if mainRecipient not in recipients: recipients.insert(0, mainRecipient)

    # get the bot credentials
    sender, sender_pass = get_email_creds()

    server  = 'smtp.gmail.com'
    
    if verbose: print('Configuring Email Headers')
    msg = MIMEMultipart()
    msg['Subject'] = subject
    msg['From'] = sender
    msg['To'] = mainRecipient
    msg['Cc'] = ( ";".join(recipients) ).replace("\n\r ", "")
    if important:
        msg['X-Priority'] = "1"
        msg['X-MSMail-Priority'] = "High"


    if verbose: print('Configuring Email Content')
    if content == "text":
        msg.attach( MIMEText(text) )
    else:
        msg.attach( MIMEText(text, content) )
    
    for filename in files:
        if verbose: print("Attaching file ({0})".format(filename))
        with open(filename, "rb") as file:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload( file.read() )
            encoders.encode_base64(part)
            part.add_header('Content-Disposition', 'attachment; filename="{0}"'.format(os.path.basename(filename)))
            msg.attach(part)

    if verbose: print(f'Connecting to SMTP server: {server}...')
    
    # connect to gmail smpt server with this port
    session = smtplib.SMTP(server, 587)#587

    #enable security
    session.starttls()

    #log in with the credentials of the bot
    session.login(sender, sender_pass)

    if verbose: print('Sending Email...')

    session.sendmail(sender, recipients, msg.as_string())
    
    if verbose: print('Sent')
    session.quit()


def send_text(text, subject, sender, mainRecipient, recipients, files=[], important=False, server="torsmtp.amd.com", content="text", verbose=False):
    return send_email(text, subject, sender, mainRecipient, recipients, files=files, important=important, server=server, content=content, verbose=verbose)