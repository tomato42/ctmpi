import random
import struct

limb_size = 8
nlimb = 32  # operate on 2048 bit numbers

# make a modulus with all bits set
mod = 2**(8*limb_size*nlimb)

probes = [
        ("null-null", (0, 0)),
        ("null-byte", (0, 0xff)),
        ("byte-null", (0xff, 0)),
        ("byte-byte", (0xff, 0xff)),
        ("qword-one", (2**(8*limb_size)-1, 1)),
        ("qword-qword", (2**(8*limb_size)-1, 2**(8*limb_size)-1)),
        ("mod-1-null", (mod-1, 0)),
        ("mod-1-one", (mod-1, 1)),
        # propagagation over all possible limbs
        ("mod-1-mod-1", (mod-1, mod-1)),
        # some carry propagation in low limbs:
        ("qqword-qqword", (2**(8*limb_size*4)-1, 2**(8*limb_size*4)-1)),
        # carry propagation in mid limbs:
        ("mid-mid", ((2**(8*limb_size*8)-1)<<(8*limb_size*4),
                     (2**(8*limb_size*8)-1)<<(8*limb_size*4))),
        # carry propagation in high limbs:
        ("high-high", (2**(8*limb_size*nlimb)-1-(2**(8*limb_size*24)-1),
                       2**(8*limb_size*nlimb)-1-(2**(8*limb_size*24)-1))),
        ]

N = 100000
log_out = "log.csv"
data_out = "data.bin"

log_file = open(log_out, "w")
data_file = open(data_out, "wb")

log_file.write(",".join(name for name, _ in probes))
log_file.write("\n")

rand = random.SystemRandom()

o = [None] * len(probes)
for _ in range(N):
    order = random.sample(range(len(probes)), len(probes))
    for i, pos in enumerate(order):
        val_a, val_b = probes[pos][1]
        data_file.write(val_a.to_bytes(length=limb_size*nlimb, byteorder="big"))
        data_file.write(val_b.to_bytes(length=limb_size*nlimb, byteorder="big"))
        data_file.write(((val_a + val_b) & (mod-1)).to_bytes(length=limb_size*nlimb, byteorder="big"))
        o[pos] = i

    # in log we need to write the position of probe in tuple, not
    # order in which they were executed
    log_file.write(",".join(str(i) for i in o))
    log_file.write("\n")

log_file.close()
data_file.close()
