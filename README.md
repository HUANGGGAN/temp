---
title: NASA_HW5

---

## 1. Setting up PowerDNS

### 1.

[1] https://mariadb.org/download/?t=repo-config&d=22.04+%22jammy%22&v=11+Rolling&r_m=blendbyte

[2] https://blog.tarswork.com/post/mariadb-install-record

[3] https://phoenixnap.com/kb/powerdns-ubuntu
1. Follow the url[1] and get mariadb repository.
```
$ apt-get install apt-transport-https curl
$ mkdir -p /etc/apt/keyrings
$ curl -o /etc/apt/keyrings/mariadb-keyring.pgp \
'https://mariadb.org/mariadb_release_signing_key.pgp'
```
2. Paste this in ```/etc/apt/sources.list.d/mariadb.sources```

```
# MariaDB 11 Rolling repository list - created 2025-03-28 15:11 UTC
# https://mariadb.org/download/
X-Repolib-Name: MariaDB
Types: deb
# deb.mariadb.org is a dynamic mirror if your preferred mirror goes offline.
# See https://mariadb.org/mirrorbits/ for details.
# URIs: https://deb.mariadb.org/11/debian
URIs: https://tw1.mirror.blendbyte.net/mariadb/repo/11.rolling/debian
Suites: bookworm
Components: main
Signed-By: /etc/apt/keyrings/mariadb-keyring.pgp
```
3. Install mariadb
```
$ apt install mariadb-server
```


4. Setup mysql_secure_installation

```
$ mysql_secure_installation 
Enter current password for root (enter for none): 
Switch to unix_socket authentication [Y/n] n
Change the root password? [Y/n] y
New password: <YOUR-PASSWORD>
Re-enter new password: <YOUR-PASSWORD>
Remove anonymous users? [Y/n] Y
Disallow root login remotely? [Y/n] Y
Remove test database and access to it? [Y/n] Y
Reload privilege tables now? [Y/n] Y
```
[4] https://doc.powerdns.com/authoritative/installation.html

[5] https://repo.powerdns.com/

5. Get powerdns repository from url[5], and find debian 12 section and follow the instructions. Install pdns-server.

![image](https://hackmd.io/_uploads/r1Jq-BNTJe.png)

Create the file ```/etc/apt/sources.list.d/pdns.list``` with this content:

```
deb [signed-by=/etc/apt/keyrings/auth-49-pub.asc] \
http://repo.powerdns.com/debian bookworm-auth-49 main
```

Put this in ```/etc/apt/preferences.d/auth-49```

```
Package: auth*
Pin: origin repo.powerdns.com
Pin-Priority: 600
```

And execute the following commands

```
$ sudo install -d /etc/apt/keyrings; 
$ curl https://repo.powerdns.com/FD380FBB-pub.asc | \
sudo tee /etc/apt/keyrings/auth-49-pub.asc &&
$ sudo apt-get update &&
$ sudo apt-get install pdns-server
```

6. Install powerdns backend.
```
$ apt-get install pdns-backend-mysql
```

7. Configure mariadb for powerdns.
```
$ mysql -u root -p

CREATE DATABASE <YOUR_DATABASE_NAME>;

GRANT ALL ON <YOUR_DATABASE_NAME>.* TO 'powerdns'@'localhost' 
IDENTIFIED BY '<YOUR_PASSWORD>';

FLUSH PRIVILEGES;

EXIT;
```
8. Import schema for powerdns.
```
$ mysql
> use <YOUR_DATABASE_NAME>;
```
```
CREATE TABLE domains (
  id                    INT AUTO_INCREMENT,
  name                  VARCHAR(255) NOT NULL,
  master                VARCHAR(128) DEFAULT NULL,
  last_check            INT DEFAULT NULL,
  type                  VARCHAR(8) NOT NULL,
  notified_serial       INT UNSIGNED DEFAULT NULL,
  account               VARCHAR(40) CHARACTER SET 'utf8' DEFAULT NULL,
  options               VARCHAR(64000) DEFAULT NULL,
  catalog               VARCHAR(255) DEFAULT NULL,
  PRIMARY KEY (id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE UNIQUE INDEX name_index ON domains(name);
CREATE INDEX catalog_idx ON domains(catalog);


CREATE TABLE records (
  id                    BIGINT AUTO_INCREMENT,
  domain_id             INT DEFAULT NULL,
  name                  VARCHAR(255) DEFAULT NULL,
  type                  VARCHAR(10) DEFAULT NULL,
  content               VARCHAR(64000) DEFAULT NULL,
  ttl                   INT DEFAULT NULL,
  prio                  INT DEFAULT NULL,
  disabled              TINYINT(1) DEFAULT 0,
  ordername             VARCHAR(255) BINARY DEFAULT NULL,
  auth                  TINYINT(1) DEFAULT 1,
  PRIMARY KEY (id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE INDEX nametype_index ON records(name,type);
CREATE INDEX domain_id ON records(domain_id);
CREATE INDEX ordername ON records (ordername);


CREATE TABLE supermasters (
  ip                    VARCHAR(64) NOT NULL,
  nameserver            VARCHAR(255) NOT NULL,
  account               VARCHAR(40) CHARACTER SET 'utf8' NOT NULL,
  PRIMARY KEY (ip, nameserver)
) Engine=InnoDB CHARACTER SET 'latin1';


CREATE TABLE comments (
  id                    INT AUTO_INCREMENT,
  domain_id             INT NOT NULL,
  name                  VARCHAR(255) NOT NULL,
  type                  VARCHAR(10) NOT NULL,
  modified_at           INT NOT NULL,
  account               VARCHAR(40) CHARACTER SET 'utf8' DEFAULT NULL,
  comment               TEXT CHARACTER SET 'utf8' NOT NULL,
  PRIMARY KEY (id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE INDEX comments_name_type_idx ON comments (name, type);
CREATE INDEX comments_order_idx ON comments (domain_id, modified_at);


CREATE TABLE domainmetadata (
  id                    INT AUTO_INCREMENT,
  domain_id             INT NOT NULL,
  kind                  VARCHAR(32),
  content               TEXT,
  PRIMARY KEY (id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE INDEX domainmetadata_idx ON domainmetadata (domain_id, kind);


CREATE TABLE cryptokeys (
  id                    INT AUTO_INCREMENT,
  domain_id             INT NOT NULL,
  flags                 INT NOT NULL,
  active                BOOL,
  published             BOOL DEFAULT 1,
  content               TEXT,
  PRIMARY KEY(id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE INDEX domainidindex ON cryptokeys(domain_id);


CREATE TABLE tsigkeys (
  id                    INT AUTO_INCREMENT,
  name                  VARCHAR(255),
  algorithm             VARCHAR(50),
  secret                VARCHAR(255),
  PRIMARY KEY (id)
) Engine=InnoDB CHARACTER SET 'latin1';

CREATE UNIQUE INDEX namealgoindex ON tsigkeys(name, algorithm);
```
```
exit;
```

[6] https://doc.powerdns.com/authoritative/settings.html#local-port

9. Create configure file ```/etc/powerdns/pdns.d/pdns.local.gmysql.conf``` and paste the following setting. Here we set local port to 5301.

```
# MySQL Configuration
#
# Launch gmysql backend
launch+=gmysql
# gmysql parameters
gmysql-host=127.0.0.1
gmysql-port=3306
gmysql-dbname=<YOUR_DATABASE_NAME>
gmysql-user=<YOUR_DATABASE_NAME>
gmysql-password=<YOUR_PASSWORD>
gmysql-dnssec=yes
local-port=5301
# gmysql-socket=
```

10. Restart powerdns
```
$ systemctl restart pdns
```

11. Check result
![image](https://hackmd.io/_uploads/SJEJBLNT1g.png)

![image](https://hackmd.io/_uploads/BJfpqyITyl.png)



### 2.

[7] https://github.com/PowerDNS-Admin/PowerDNS-Admin/blob/master/docs/wiki/install/Running-PowerDNS-Admin-on-Ubuntu-or-Debian.md

1. Follow the instruction in the url[7] and install required packages
```
$ sudo apt install -y python3-dev git libsasl2-dev libldap2-dev python3-venv \
libmariadb-dev pkg-config build-essential curl libpq-dev  libxmlsec1-dev
```

Here we change the version of NodeJs for security(the version is too old in url[7]).

```
$ curl -sL https://deb.nodesource.com/setup_18.x | sudo bash -
$ sudo apt install -y nodejs
```

```
$ curl -sL https://dl.yarnpkg.com/debian/pubkey.gpg | gpg --dearmor | \
sudo tee /usr/share/keyrings/yarnkey.gpg >/dev/null

$ echo "deb [signed-by=/usr/share/keyrings/yarnkey.gpg] \
https://dl.yarnpkg.com/debian stable main" | \
sudo tee /etc/apt/sources.list.d/yarn.list

$ sudo apt update && sudo apt install -y yarn
```

2. Clone the repository to local(here we choose the same directory as in url[7]), and create virtual environment for python.

```
$ git clone https://github.com/PowerDNS-Admin/PowerDNS-Admin.git \
/opt/web/powerdns-admin
$ cd /opt/web/powerdns-admin
$ python3 -mvenv ./venv
```

3. Start the environment and install some necessary requirements.

```
$ source ./venv/bin/activate
$ pip install --upgrade pip
$ pip install -r requirements.txt
```

4. Set up configurations for powerdns
```
$ cp /opt/web/powerdns-admin/configs/development.py \
/opt/web/powerdns-admin/configs/production.py
```

In ```/opt/web/powerdns-admin/configs/production.py```, replace the following configuration.

```
### BASIC APP CONFIG
SECRET_KEY = '<NEW_GENERATED_KEY>'

### DATABASE CONFIG
SQLA_DB_USER = '<YOUR_DATABASE_NAME>'
SQLA_DB_PASSWORD = '<YOUR_PASSWORD>'
SQLA_DB_NAME = '<YOUR_DATABASE_NAME>'
```

[8] https://flask.palletsprojects.com/en/stable/config/#SECRET_KEY
For ```SECRET_KEY```, use

```
$ python -c 'import secrets; print(secrets.token_hex())'
```

5. Database migration

```
$ export FLASK_CONF=../configs/production.py
$ export FLASK_APP=powerdnsadmin/__init__.py
$ flask db upgrade
```

6. Build assets

```
$ yarn install --pure-lockfile
$ flask assets build
```

7. Run PowerDNS admin with ```./run.py``` and connect to ```localhost:9191```

8. Sign up and log in.

![image](https://hackmd.io/_uploads/Sk4AuP4pkg.png)

9. Whenever you want to startup, do

```
$ cd /opt/web/powerdns-admin
$ source venv/bin/activate
$ export FLASK_APP=powerdnsadmin/__init__.py
$ flask db upgrade
$ ./run.py
```

### 3.

[9] https://noob.tw/nginx-reverse-proxy/

1. Install nginx.

```
$ apt install nginx
```

2. Configure nginx in file```/etc/nginx/sites-enabled/default```, and modify this part.
![image](https://hackmd.io/_uploads/r160zzHT1g.png)

3. Reload service

```
$ nginx -t # Check syntax error
$ systemctl restart nginx
```

4. Resolve hostname(b12902039.com) on host, add the line in ```/etc/hosts```

```
127.0.0.1 b12902039.com
```

5. Check result, ensure the following service is running. Connect to ```b12902039.com```
```
1. mysql           # systemctl status mysql
2. nginx           # systemctl status nginx
3. pdns.service    # systemctl status pdns
4. powerdns-admin  # ./run.py
```
![image](https://hackmd.io/_uploads/B13z4zr6ke.png)

### 4.

1. Setup API keys. Find API Keys section on the left and create a key for administrator. Copy <YOUR_API_KEY>.

![image](https://hackmd.io/_uploads/BktZ8XHTJl.png)

2. Go to ```/etc/powerdns/pdns.conf``` and modify this part

```
api=yes
api-key=<YOUR_API_KEY>
webserver=yes
```

Note that the api key is different from the one in ```/opt/web/powerdns-admin/configs/production.py```.

3. Create a zone "cscat.tw".

![image](https://hackmd.io/_uploads/HkRRjmSayl.png)

4. Add records.

![image](https://hackmd.io/_uploads/HJdSZESp1x.png)

5. Check results.

![image](https://hackmd.io/_uploads/SJOBoyLakx.png)

![image](https://hackmd.io/_uploads/By4vsJ8TJx.png)

![image](https://hackmd.io/_uploads/SkPoikUpkx.png)

![image](https://hackmd.io/_uploads/ryypjyLpke.png)

![image](https://hackmd.io/_uploads/Sk2Rs1UT1g.png)

![image](https://hackmd.io/_uploads/H1n131Ipyx.png)

![image](https://hackmd.io/_uploads/H1Nbnk86ke.png)


### 5.

[10] https://www.youtube.com/watch?v=Fk2oejzgSVQ

![image](https://hackmd.io/_uploads/H1OB4NHpyl.png)

DNSSEC (Domain Name System Security Extensions): Aims to provide security for DNS, prevent from the attackers disguising as the target server.

DNSSEC uses the following keys/record to show their identity via digital signature.

- KSK (Key Signing Key):
Used to sign the DNSKEY record.
- ZSK (Zone Signing Key):
Used to sign the RRsets.
- DNSKEY record:
Contains the public keys of both the KSK and the ZSK, and is signed by the KSK.
- RRsets (Resource Record Sets):
These are the actual DNS records (A, AAAA, MX, TXT, etc.).
- RRsig records:
These are the digital signatures corresponding to each RRset, signed by the ZSK.
- DS record (Delegation Signer):
This is the hash of the child zone's KSK, and is placed in the parent zone.

Now when a resolver requests one of the root server. The root server provides,

1. DNSKEY record
2. RRsig of DNSKEY record(signed by KSK)
3. DS record
4. RRsig of DS record(signed by ZSK)

Once the root server's KSK obtained by other means(such as already containing in operating system), we can use (2) to verify (1) and get pubZSK. Once obtain pubZSK, we can verify (4) by (3), and hence know the child zone's server.

## 2. PowerDNS Recursor

### 0. Basic

#### 1. 

Authoritive server provides answers that are configured in its system to DNS queries without asking other server.

#### 2.

- Recursive DNS query
Ask the server to get the question by query on any other server.
- Iterative DNS query
Ask the server to provide information but do not ask any other server.

### 1. Setting up PowerDNS Recursor

#### 1.

[11] https://doc.powerdns.com/recursor/getting-started.html

[12] https://doc.powerdns.com/recursor/settings.html

1. Install pdns-recursor

```
$ apt install pdns-recursor
```

2. Configure in ```/etc/powerdns/recursor.conf```

```
local-address=127.0.0.1, ::1
local-port=10053
max-cache-ttl=300
forward-zones-file=/etc/powerdns/forward-zones.conf
forward-zones-recurse=.=8.8.8.8
dnssec=validate
```

3. Set```/etc/powerdns/forward-zones.conf```
```
cscat.tw=127.0.0.1:5301
```

4. Restart service.

```
$ systemctl restart pdns-recursor
```

#### 2.

![image](https://hackmd.io/_uploads/rJv3MTrT1x.png)

#### 3.

![image](https://hackmd.io/_uploads/H1N0Gprake.png)

8.8.8.8 is fully DNSSEC-validating and can resolve google.com successfully, the recursor will receive validly signed answer from google.
However, the authoritative server(on port 5301), is not known by public, that is, the KSK is not known by others, the authentation failed naturally.

#### 4.

[13] https://doc.powerdns.com/recursor/dnssec.html

[14] https://doc.powerdns.com/recursor/lua-config/dnssec.html

1. Get your DS record(choose one).

![image](https://hackmd.io/_uploads/BkfiTaBp1l.png)

2. Configure ```/etc/powerdns/recursor.lua``` add the following line.

```
addTA("cscat.tw", "<YOUR_DS_RECORD>")
```

3. Restart service and check result

![image](https://hackmd.io/_uploads/H1HvRpBayg.png)


#### 5.

[15] https://bluecatnetworks.com/blog/for-dns-server-caching-what-is-the-ideal-ttl/

- If TTL too high, the user may get expired information
- If TTL too low, the server will frequently update its data and lower the efficiency of the service

#### 6.

[16] https://www.cloudns.net/blog/authoritative-dns-server/

1. Attacker send query to recursor, trigger the query on authoritative server.
2. Attacker uses plenty of fake response, pretending it is the authoritative(since DNS use UDP, it cannot tell the identity of the real authoritative server unless using DNSSEC)
3. The recursor get the wrong information, especially NS record for a certain zone, which point to a fake server from attacker.
4. Recursor store the fake information in cache.
5. When others send query to recursor asking the domain name in the zone, they get the wrong nameserver, which is own by attacker.
6. Users get wrong IP and may reveal their important data to attacker.

### 2. Security

#### 1.

[17] https://www.tenable.com/audits/items/CIS_ISC_BIND_DNS_Server_9.11_Benchmark_v1.0.0_L1_CachingOnly.audit:fdccdfb7b846f191c19bba508040821a

[18] https://www.f5.com/glossary/dns-flood-nxdomain-flood

If allow non-trusted user do recursive query, they may continuous send 
requests for invalid or nonexistent records, which can make the recursor keep asking the answer and get the server fill the negative responses.

#### 2.

1. Configure ```/etc/powerdns/recursor.conf```, (Convert 192.168.122.0/24 to your internal network).

```
allow-from=192.168.122.0/24
```

2. Restart service

## 3. dnsdist

### 1. Setting up dnsdist

#### 1.

[19] https://www.dnsdist.org/index.html

1. Install dnsdist

```
$ apt-get install -y dnsdist
```

2. Create a configure file ```dnsdist.conf```

```
newServer("8.8.8.8")
newServer("127.0.0.1:10053")
```

3. Run dnsdist on port 53(make sure no one is using 53)

```
$ dnsdist -C dnsdist.conf --local=0.0.0.0:53
```

4. Check result(if chose 8.8.8.8, dig cscat.tw will not success).

![image](https://hackmd.io/_uploads/ryHg6yUpke.png)


#### 2.

[20] https://www.dnsdist.org/advanced/qpslimits.html

[21] https://www.dnsdist.org/reference/selectors.html#DNSRule

1. Add the following rules to the config file ```dnsdist.conf```, if version > 1.9.0, check url[18] for new syntax.

```
addAction(AndRule({ QNameWireLengthRule(0, 70), QTypeRule(DNSQType.TXT) }), DropAction())
addAction(MaxQPSIPRule(20), DropAction())

smn = newSuffixMatchNode()
smn:add("csdog.tw.")
addAction(SuffixMatchNodeRule(smn), DropAction())
```

### 2. DNS-over-TLS

#### 1.

[22] https://en.wikipedia.org/wiki/DNS_over_TLS

[23] https://www.cloudflare.com/zh-tw/learning/dns/dns-over-tls/

- DNS over TLS (DoT) is for encrypting DNS query and answer by TLS protocol, preventing man-in-the-middle attack and increase user privacy.



| Column 1 | DoT | DoH |
| -------- | -------- | -------- |
| 加密機制     | uses SSL/TLS    | uses HTTPS     |
| 效能    | faster, DNS runs on TLS directly    | slower, use HTTP, bigger packet size, and send with other non-DNS packets     |
| 安全性    | better security, can identify malicious traffic easier     | better privacy, hide in HTTPS     |



#### 2.

[24] https://blog.cssuen.tw/create-a-self-signed-certificate-using-openssl-240c7b0579d3

1. Create a certificate

```
openssl req -x509 -newkey rsa:4096 -sha256 -nodes \
-keyout /etc/powerdns/dnsdist_key.pem \
-out /etc/powerdns/dnsdist_cert.pem -days 30
```

2. Add to configure file

```
addTLSLocal('127.0.0.1', '/etc/powerdns/dnsdist_cert.pem', \
'/etc/powerdns/dnsdist_key.pem')  # In a single line
```

#### 3.
![image](https://hackmd.io/_uploads/B1iiOkUa1l.png)
## 4. Master and Slave

### 1.

- 如果今天其中一台伺服器壞掉了怎麼辦？
So we need at least 2 servers for each service.
- 如果今天系館停電導致所有機房下線怎麼辦？
So servers of same service should be in at least 2 different place.
- 如果因為某些原因導致伺服器上的 DNS records 不見了怎麼辦？
So the database(MariaDB) should do backup.

Layout:

- 2 Authoritative Nameserver, 1 at 系上, 1 at 計中:
When csie server is down, DNS query will automaticly query the one at 計中
- 3 MariaDB server, 1 at 系上, 1 at 計中 1 on the cloud:
More server for data storage, since data lost is more serious.
Set master and slave, when one is down, the other can become master.
Backup to cloud.
- 2 Recursor, 1 at 系上, 1 at 計中:
Same as Authoritative server
- 2 PowerDNS Admin, 1 at 系上, 1 at 計中:
Same as Authoritative server
- 2 dnsdist, 1 at 系上, 1 at 計中:
The one at 計中 can handle more query on external network(not *.csie.ntu.edu.tw) since it is closer to outside.

With this setup, we can avoid the 3 problem above most of the time.

### 2.

[25] https://developers.cloudflare.com/dns/zone-setups/zone-transfers/


When syncroizing primary and secondary DNS servers

| Column 1 | AXFR | IXFR |
| -------- | -------- | -------- |
| Transter     | Whole zone file    | Only transfers the changes     |
|Efficiency| Slower when in large scale| Faster  |
| When to use| Adding a new authoritative server | Already have syncronized data |

IXFR use case: Most of the time.
AXFR use case: Adding a new server, Rollback or Recovery, migrating.

