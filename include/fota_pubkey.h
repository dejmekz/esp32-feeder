#ifndef FOTA_PUBKEY_H
#define FOTA_PUBKEY_H

// Public half of the RSA-4096 keypair used to sign OTA firmware images.
// esp32FOTA verifies this signature before switching to a downloaded
// image, so a malicious or corrupted firmware.img (e.g. from a MITM on
// the network) is rejected even though the manifest/firmware are
// fetched over plain HTTP.
//
// The matching private key (keys/priv_key.pem, gitignored) never leaves
// the machine used to sign releases. See README.md "Firmware signing"
// for how to generate a keypair and sign a build.
const char *FOTA_RSA_PUB_KEY PROGMEM = R"KEY(
-----BEGIN PUBLIC KEY-----
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAtMZ3J3CDbAstqQTIui70
S6Y3CpalbAIFulQn6nEqGKM0U2UD4+/GUlrdRu8zItvjX+uTK5t7Pntbu5geHM3N
fEHqjlza22jvMN9X293FqEw9bxnKTEROiqYO32xlOuLD7aGYDua6UAfcZacfhyC2
AXKslbopFdSpt5+feWNTSvu07mFe2wnlJ883KFkwF2j4ajvPCIeNp2/wCPNBIeLh
Fog+TB3aLyT7MprfHfUBE/3jDOkTlk9qKIo5aLONkKE8jWMpiJgqRU6eZ4ia7suP
UuVwMhoz/j8hkEJmpWXXEA8tThzoDDrrov+IRPiw7QZVxUCHLVK1W9c7mpOPpaGw
su3buOABxo8Mv5KTvg7OzGq4u4PCeQYZTql//nyGZeO52p61rcA110Wwt0J4jF7h
c2SDBZg0CbU+slUMjtou3k2T1zhJ5SMRGlIuN9ovVxwPG0p95geYLdy1a6xaYk4A
l8VJjnSkZ/TNDUJtNNoVzjdTUAIKAsph7O1pDbOarThlaHesaLpWhLFucfP6l94b
tKxZwyZLhGK2tHBqUui8Lz1c0j5c9T4Sg5vseyBgkmC/4sC3VwOCNZ6GOcuGP0yE
E6ivK7fv8YagTDKpLuO/NP9vZ0gOK1di/pbwSWC71RVONQSz5HltcQQxgRYRyo1f
UScBjdrpMyXnaRwzmAXeXeMCAwEAAQ==
-----END PUBLIC KEY-----
)KEY";

#endif
