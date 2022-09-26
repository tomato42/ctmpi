import random
import struct
import sys
import getopt

def help_msg():
    print("Usage: {0} [options]".format(sys.argv[0]))
    print("")
    print(" -N num    Number of test tuples to generate, default: 100k")
    print(" -b num    Number of bytes in a machine word for the SUT")
    print("           Use \"4\" for 32 bit systems and \"8\" for 64 bit")
    print("           systems. Default: 8")
    print(" -n num    Number of machine words to use for generated numbers")
    print("           Default: 32 (i.e. 2048 bit on 64-bit machine)")
    print(" -l file   Name of output log file, \"log.csv\" by default")
    print(" -o file   Name of output file with generated data.")
    print("           \"data.bin\" by default")
    print(" --add     Generate data for addition tests (default)")
    print(" --sub     Generate data for subtraction tests")
    print(" -h        This message")

limb_size = 8
nlimb = 32  # operate on 2048 bit numbers

N = 100000
log_out = "log.csv"
data_out = "data.bin"
oper = "add"

log_file = open(log_out, "w")
data_file = open(data_out, "wb")

argv = sys.argv[1:]

opts, args = getopt.getopt(argv, "N:b:n:l:o:h", ["add", "sub"])
for opt, arg in opts:
    if opt == "-h":
        help_msg()
        sys.exit(0)
    elif opt == "-N":
        N = int(arg)
    elif opt == "-b":
        limb_size = int(arg)
    elif opt == "-n":
        nlimb = int(arg)
    elif opt == "-l":
        log_out = arg
    elif opt == "-o":
        data_out = arg
    elif opt == "--add":
        oper = "add"
    elif opt == "--sub":
        oper = "sub"
    else:
        print("Unrecognised option: {0}".format(opt))
        help_msg()
        sys.exit(1)

if args:
    print("Trailing arguments: {0}".format(args))
    help_msg()
    sys.exit(1)

# make a modulus the count of numbers that can be represented in this many bits
mod = 2**(8*limb_size*nlimb)

print(hex(mod-1))

probes = [
        ("null-null", (0, 0)),
        ("null-byte", (0, 0xff)),
        ("byte-null", (0xff, 0)),
        ("byte-byte", (0xff, 0xff)),
        ("qword-one", (2**(8*limb_size)-1, 1)),
        ("qword-qword", (2**(8*limb_size)-1, 2**(8*limb_size)-1)),
        ("mod-1-null", (mod-1, 0)),
        ("null-mod-1", (0, mod-1)),
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

log_file.write(",".join(name for name, _ in probes))
log_file.write("\n")

rand = random.SystemRandom()

o = [None] * len(probes)
for _ in range(N):
    order = random.sample(range(len(probes)), len(probes))
    for i, pos in enumerate(order):
        val_a, val_b = probes[pos][1]
        assert val_a < mod
        assert val_b < mod
        data_file.write(val_a.to_bytes(length=limb_size*nlimb, byteorder="big"))
        data_file.write(val_b.to_bytes(length=limb_size*nlimb, byteorder="big"))
        if oper == "add":
            data_file.write(((val_a + val_b) & (mod-1)).to_bytes(length=limb_size*nlimb, byteorder="big"))
        else:
            assert oper == "sub"
            data_file.write(((val_a - val_b) & (mod-1)).to_bytes(length=limb_size*nlimb, byteorder="big"))
        o[pos] = i

    # in log we need to write the position of probe in tuple, not
    # order in which they were executed
    log_file.write(",".join(str(i) for i in o))
    log_file.write("\n")

log_file.close()
data_file.close()
