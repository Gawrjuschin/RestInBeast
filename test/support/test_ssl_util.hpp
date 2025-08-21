#ifndef TEST_SSL_UTIL_HPP
#define TEST_SSL_UTIL_HPP

#include <boost/asio/ssl.hpp>

namespace test {
/**
 * @brief make_server_ssl_ctx
 */
inline auto make_server_ssl_ctx() {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test server's cert signed by CA:
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  constexpr std::string_view signed_server_cert = R"(
-----BEGIN CERTIFICATE-----
MIIDyjCCArKgAwIBAgIUN26BTikliJkmTHmUZgMyxEK/h2MwDQYJKoZIhvcNAQEL
BQAwfjELMAkGA1UEBhMCUlUxDzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9z
Y293MQwwCgYDVQQKDANFZHUxGzAZBgNVBAMMEkRtaXRyaXkgR2F2cnl1c2hpbjEi
MCAGCSqGSIb3DQEJARYTbW9ydGFuZWdnQGdtYWlsLmNvbTAeFw0yNTA3MDkyMjM2
NTRaFw0yNzA3MDkyMjM2NTRaME8xCzAJBgNVBAYTAlJVMQ8wDQYDVQQIDAZNb3Nj
b3cxDzANBgNVBAcMBk1vc2NvdzEMMAoGA1UECgwDRWR1MRAwDgYDVQQDDAdEbWl0
cml5MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAo54gEiMi0P8UYcMw
pcur29rMX4OAGwwe3X3ADVy3BFHe7j1EXmyWdwbjqBF8I5axm5qvCWdzXsLXu/jl
qCVjLYzn5jruVGcbo4vrJr+P9R1VS98GjTr+rPUOeLYZbL398SZc/OaQndDu7bRC
Jnjw7FyU6AuGnigbi47oqf9UaAMB02GgqRtSR6qPJe8in+Pyb26v/VpyH+RO9i3I
VwnEt8YboHdLmVJDTGBWMOE70tNtmhS7uC9Y8eiFBg8EK1/nwdMzWr06C1dNcXTv
6N4c0Bqjo8ScxgTuZto84avAu3Vj6kyyNGuOhr3pzW8QAkhBDdAf4x2m/QkU1zig
bFucnwIDAQABo28wbTAfBgNVHSMEGDAWgBQPFtTkb2FrRoirmGNRGUsQvTgrWDAJ
BgNVHRMEAjAAMAsGA1UdDwQEAwIE8DATBgNVHREEDDAKgghyaWIudGVzdDAdBgNV
HQ4EFgQUS7HWSx9L9nu+5vO2hFJX/6jGFSEwDQYJKoZIhvcNAQELBQADggEBANWV
ymU6fDhQObTKSYY2Png8aNyXYdcV9nzC+W/VlKqmyYl6NInpZi05Ht+qxMpplB4j
MFhTVQMxhueO06IgQRAZhDmHkZeSYiSSspvWuN4CR4TP37JRbb9SXVdFYYGGWuZr
Qns7C/et8wcPRqyUNy4QmWEUrM3LPDJCxsobWK4YT67/Dmi7yHlArTnYgbCbeC3u
76BVDycbfknZ5FcTP7xecSfrSjH1jajdKnEbb4xoScFvTUl0N/KI0lWnGt2nCe8U
BKrVabcobI8kgMZWH4amzrae7MrOymcqn5bIj3e2gtPU/kwZUzhR04+DO2Tuw13B
/IVH+F5C1ZlIEK8JsqA=
-----END CERTIFICATE-----
)";
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test server's private key:
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  constexpr std::string_view server_private_key = R"(
-----BEGIN PRIVATE KEY-----
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQCjniASIyLQ/xRh
wzCly6vb2sxfg4AbDB7dfcANXLcEUd7uPURebJZ3BuOoEXwjlrGbmq8JZ3Newte7
+OWoJWMtjOfmOu5UZxuji+smv4/1HVVL3waNOv6s9Q54thlsvf3xJlz85pCd0O7t
tEImePDsXJToC4aeKBuLjuip/1RoAwHTYaCpG1JHqo8l7yKf4/Jvbq/9WnIf5E72
LchXCcS3xhugd0uZUkNMYFYw4TvS022aFLu4L1jx6IUGDwQrX+fB0zNavToLV01x
dO/o3hzQGqOjxJzGBO5m2jzhq8C7dWPqTLI0a46GvenNbxACSEEN0B/jHab9CRTX
OKBsW5yfAgMBAAECggEAGGAQ0BLq927g9vWQ1Epk9pKAmwQovB7QpFe9GmCDJYIa
76wx+M09l0rv9JRyRY0INbHUPNbsIaRuZmMct7GmNl2FGNATvIBF5lYMeM/WHwK5
N93oiHbPBS5W8xXR8NjXMatw8/5XciNzzPbrWfbvkY7zYiSBcCNygxaNw/G1fe8j
kBIqkjOWJFf65GEJJ1k0n01y3b/hMwZ796M2r2nBBfpeasX/STVamr1AEnj0u2Uk
TTAgK9vAxsAHoaRGXtlrptjXs6g6SeSEIyAmodqWWlWfOcNjiWASsQODTuQC5mdF
MypEQTFwayKqGiew5ZCQ1ntbcNhYClViBAJNZTN9qQKBgQDeZKwOditQnR9kvmDI
X729g7yoQAMIO8C9aslLbYxVrp+3dzyMa/rVe/yUpaYRw6afEDyOo5Za/LcVAtvA
HYGPpVoV/spo3iBi8i/saW4kPApuCiuzAIaO8sU4lo56mBYBn7Bh+8SWz8v3iNYO
tebB3c8H0xU/D/p+U/8BxpfwiQKBgQC8V7VOHzdDhRJnG5XPsYdXAbE2hjGyhfpM
XAwhfgqkzp3M5L6r88+ZwahkYMRUVHpjDbhqBDLtPgz+pTd8qP396DQWxya4oaZn
Mj2PdenqURJU5Y3Qb5f89+yuiwfU1+S/84wwqpziEWb8lx4mP7evU426QUDtKKhE
lzkBjLbJ5wKBgQDXhYiYmzxu61Y5M6ZniSFAQCLciuxlVkL7Tjm1t3yArLJsAknT
a/GyYm3tQHsoTAt1qy6ErVRQ6QgkfJiqHe4pX31mQ321E08flMWMvr/WrbkN2x1u
Y9wDPJKUvJNni2wINtGgo3VpEggA48NOjkoLJwIz+wRkEQAY3R4MgoFMwQKBgQC7
YT9+U8w1nsUqU+V+Vw9hXsuihWwYQvc81S80eyO3x+BzIvkYNl9Kh8My6pspk0qY
FeRthXPtWJixGErV+YOJA2Yfa094zUqyzOc2PYfVmYb1c9TdlMjP4xFUrfnCpnBt
EK9fE781oz6k+uwB/c6P3j7ZGWaxDIwsgY8znX2V7QKBgQCfWhzIRtRRwMp2cqX5
AkSpd7+cikn4CVMbkwSegwIO0MrPLThy6iyYtWFytYHSvuZmDmev6SmsM0tqnyJB
lSEehEaD2XwIDrl7JNbjuMK6bK8KgLbrJiGdtT5bJafMia4XIvod0qntUJOndhT5
8L4itZJhJYIttg9AXsCj5FYKcQ==
-----END PRIVATE KEY-----
)";
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test server's dh params:
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  constexpr std::string_view server_dh_params = R"(
-----BEGIN DH PARAMETERS-----
MIIBDAKCAQEA9+pYyrC9W9FKAsNF47uPdV3ZgYcVirC7Eb9OLp2svXF5GTHDuaSK
TVaG1VsWmyfRXdgAUoybSJFfLWwfpt8dvWGu2W/FRVQZWaPRtgiaARDxm+H/esv+
Iwc6ozdRNOI7mQZ1aU9P9SewLaGrJFKdyMn0GWl1AlIfQTyHZU9r75QaW9HA6eEV
prAqfrYEJBKD0YwOHgkXoLV38Tcsx+WD9Kryb9HE+yW7QU368d9soNO34//8F/DO
dG8tvLafDn2my/cfy4E+SEY1dppT79rQbzxFJnMYvcM3x8nEUFaCnvmTu6r8HsdJ
ObPCA01YwgjK9CBlqFsXU3ry3ADgtYqaTwIBAgICAOE=
-----END DH PARAMETERS-----
)";
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::tlsv13};

  ssl_ctx.set_options(boost::asio::ssl::context::default_workarounds |
                      boost::asio::ssl::context::no_sslv2 |
                      boost::asio::ssl::context::single_dh_use);

  ssl_ctx.use_certificate(
      boost::asio::const_buffer{std::data(signed_server_cert),
                                std::size(signed_server_cert)},
      boost::asio::ssl::context::file_format::pem);

  ssl_ctx.use_private_key(
      boost::asio::const_buffer{std::data(server_private_key),
                                std::size(server_private_key)},
      boost::asio::ssl::context::file_format::pem);

  ssl_ctx.use_tmp_dh(boost::asio::const_buffer{std::data(server_dh_params),
                                               std::size(server_dh_params)});

  ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);

  return ssl_ctx;
}
/**
 * @brief make_client_ssl_ctx
 */
inline auto make_client_ssl_ctx() {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test CA's cert:
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  constexpr std::string_view ca_cert = R"(
-----BEGIN CERTIFICATE-----
MIID3TCCAsWgAwIBAgIUCPU7zrv/ShTVcIMl88yDFuAwL1kwDQYJKoZIhvcNAQEL
BQAwfjELMAkGA1UEBhMCUlUxDzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9z
Y293MQwwCgYDVQQKDANFZHUxGzAZBgNVBAMMEkRtaXRyaXkgR2F2cnl1c2hpbjEi
MCAGCSqGSIb3DQEJARYTbW9ydGFuZWdnQGdtYWlsLmNvbTAeFw0yNTA3MDkyMjI3
MzVaFw0zNTA3MDcyMjI3MzVaMH4xCzAJBgNVBAYTAlJVMQ8wDQYDVQQIDAZNb3Nj
b3cxDzANBgNVBAcMBk1vc2NvdzEMMAoGA1UECgwDRWR1MRswGQYDVQQDDBJEbWl0
cml5IEdhdnJ5dXNoaW4xIjAgBgkqhkiG9w0BCQEWE21vcnRhbmVnZ0BnbWFpbC5j
b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDmUyeZWJjl8UfkQgA8
dve/33mIhF5CJ0jTyjotuW4tiUjivbaYuOb0YmfsBFxal0M9H5IVh+CNe/8upvT/
CrVaz/fo2yp/ZE6UhF5opztls/jzJ3FyxqjpiPvgU3acWlqWP5ptzoKD9wYdiMbZ
TKYPDqIXED8blCdDm3TPf2FBZQlGl721gPL2iqso84mkjYqVHsR5VHWdouWDyQmY
9by7yA65JZfQPBSwx17yS8puz6/D1b47QIvcjtpSYmgA76uH/XvhJuDpPp/VJQWT
0Ou3JCCpi1mHgPmHXHvLP2/6NBEQSsfn64UAMBgAegXIvj17S3VmwrBweN/N3d24
kSvDAgMBAAGjUzBRMB0GA1UdDgQWBBQPFtTkb2FrRoirmGNRGUsQvTgrWDAfBgNV
HSMEGDAWgBQPFtTkb2FrRoirmGNRGUsQvTgrWDAPBgNVHRMBAf8EBTADAQH/MA0G
CSqGSIb3DQEBCwUAA4IBAQCLZI4IjQOwiHOBbX274Z1OsGusPRggAqZYIZ00x7m8
zCJjvM+CFNi87JjpjoNES0H0+JTZgsYkpkLIgmzNuU+q3eBn/8+zJDvcaHcvvdxF
n7iIKZbn/Yq7Jwo8tidRnFVyPEnxaK9fC3FEqGRVAu8GMSrVs8e8QvHJfeUGoN7G
xofgjP6y+5qfIzLiymoqokBh18k1v6LExFgrLEaqiwJYBu+pCm6GM3O6W+anYTCW
dNTcxAFDjImO3rO6I7P3Olva21Nt5PXr4nN08r9cQxaO+nqQ+xVAeh4Nl71e/XGc
ggabvlenQdhRvl18Z2/gIBEVIt6yButG8yrR6UhoqMUN
-----END CERTIFICATE-----
)";
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::tlsv13};

  ssl_ctx.add_certificate_authority(
      boost::asio::const_buffer{std::data(ca_cert), std::size(ca_cert)});

  // Only server with cert signed by Test CA is accepted
  ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer |
                          boost::asio::ssl::verify_fail_if_no_peer_cert);

  return ssl_ctx;
}

inline auto make_client_fake_ca_ssl_ctx() {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test fake CA's cert:
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  constexpr std::string_view ca_cert = R"(
-----BEGIN CERTIFICATE-----
MIIDyjCCArKgAwIBAgIUN26BTikliJkmTHmUZgMyxEK/h2MwDQYJKoZIhvcNAQEL
BQAwfjELMAkGA1UEBhMCUlUxDzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9z
Y293MQwwCgYDVQQKDANFZHUxGzAZBgNVBAMMEkRtaXRyaXkgR2F2cnl1c2hpbjEi
MCAGCSqGSIb3DQEJARYTbW9ydGFuZWdnQGdtYWlsLmNvbTAeFw0yNTA3MDkyMjM2
NTRaFw0yNzA3MDkyMjM2NTRaME8xCzAJBgNVBAYTAlJVMQ8wDQYDVQQIDAZNb3Nj
b3cxDzANBgNVBAcMBk1vc2NvdzEMMAoGA1UECgwDRWR1MRAwDgYDVQQDDAdEbWl0
cml5MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAo54gEiMi0P8UYcMw
pcur29rMX4OAGwwe3X3ADVy3BFHe7j1EXmyWdwbjqBF8I5axm5qvCWdzXsLXu/jl
qCVjLYzn5jruVGcbo4vrJr+P9R1VS98GjTr+rPUOeLYZbL398SZc/OaQndDu7bRC
Jnjw7FyU6AuGnigbi47oqf9UaAMB02GgqRtSR6qPJe8in+Pyb26v/VpyH+RO9i3I
VwnEt8YboHdLmVJDTGBWMOE70tNtmhS7uC9Y8eiFBg8EK1/nwdMzWr06C1dNcXTv
6N4c0Bqjo8ScxgTuZto84avAu3Vj6kyyNGuOhr3pzW8QAkhBDdAf4x2m/QkU1zig
bFucnwIDAQABo28wbTAfBgNVHSMEGDAWgBQPFtTkb2FrRoirmGNRGUsQvTgrWDAJ
BgNVHRMEAjAAMAsGA1UdDwQEAwIE8DATBgNVHREEDDAKgghyaWIudGVzdDAdBgNV
HQ4EFgQUS7HWSx9L9nu+5vO2hFJX/6jGFSEwDQYJKoZIhvcNAQELBQADggEBANWV
ymU6fDhQObTKSYY2Png8aNyXYdcV9nzC+W/VlKqmyYl6NInpZi05Ht+qxMpplB4j
MFhTVQMxhueO06IgQRAZhDmHkZeSYiSSspvWuN4CR4TP37JRbb9SXVdFYYGGWuZr
Qns7C/et8wcPRqyUNy4QmWEUrM3LPDJCxsobWK4YT67/Dmi7yHlArTnYgbCbeC3u
76BVDycbfknZ5FcTP7xecSfrSjH1jajdKnEbb4xoScFvTUl0N/KI0lWnGt2nCe8U
BKrVabcobI8kgMZWH4amzrae7MrOymcqn5bIj3e2gtPU/kwZUzhR04+DO2Tuw13B
/IVH+F5C1ZlIEK8JsqA=
-----END CERTIFICATE-----
)";
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::tlsv13};

  ssl_ctx.add_certificate_authority(
      boost::asio::const_buffer{std::data(ca_cert), std::size(ca_cert)});

  // Only server with cert signed by Test CA is accepted
  ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer |
                          boost::asio::ssl::verify_fail_if_no_peer_cert);

  return ssl_ctx;
}

} // namespace test

#endif // TEST_SSL_UTIL_HPP
