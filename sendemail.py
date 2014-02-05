# coding=utf-8

# Copyright (C) 2014  Stefano Guglielmetti

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import smtplib, os, sys
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders

#From address, to address, subject and message body
from_address    = 'EMAIL_FROM_ADDRESS'
to_address      = ['EMAIL_TO_ADDRESS']
email_subject   = 'Alert!!! Zombies!!! Ahead!!!'
email_body      = 'An intruder has been detected and needs to be eliminated!'

# Credentials (if needed)
username = 'YOUR_EMAIL_USERNAME'
password = 'YOUR_EMAIL_PASSWORD'

# The actual mail send
server = 'smtp.gmail.com:587'

def send_mail(send_from, send_to, subject, text, files=[], server="localhost"):
    assert type(send_to)==list
    assert type(files)==list

    msg = MIMEMultipart()
    msg['From'] = send_from
    msg['To'] = COMMASPACE.join(send_to)
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = subject

    msg.attach( MIMEText(text) )

    for f in files:
        part = MIMEBase('application', "octet-stream")
        part.set_payload( open(f,"rb").read() )
        Encoders.encode_base64(part)
        part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(f))
        msg.attach(part)

    smtp = smtplib.SMTP(server)
    smtp.starttls()
    smtp.login(username,password)
    smtp.sendmail(send_from, send_to, msg.as_string())
    smtp.close()

send_mail(from_address, to_address, email_subject, email_body, [sys.argv[1]], server) #the first command line argument will be used as the image file name



