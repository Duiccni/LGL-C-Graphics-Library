import matplotlib.image as mpimg
image_path = "E:\\OLD_I_AKA_PROJECTS\\Python\\New\\font1.png"
image = mpimg.imread(image_path)

image_size = image.shape

print(image_size)

next_start = 0

def getdim() -> tuple[int, int]:
    global next_start
    x = 0
    y = 0

    while int(image[y][next_start][2] * 255) != 21: y += 1
    while int(image[0][next_start + x][2] * 255) != 21: x += 1

    return (x, y)

file = open("font.bin", "wb")

bytes = []

mx = 0
my = 0

sizeee = 0

while next_start < image_size[1]:
    dim = getdim()
    print(dim)
    mx = max(mx, dim[0])
    my = max(my, dim[1])
    bytes.append(dim[0])
    bytes.append(dim[1])
    sizeee += dim[0] * dim[1] + 4
    bytes.append(0)
    bytes.append(0)

    for y in range(dim[1] - 1, -1, -1):
        for x in range(next_start, next_start + dim[0]):
            if (image[y][x][0] == 0 and image[y][x][2] == 0):
                bytes.append(1)
            else:
                bytes.append(0)

    next_start += dim[0] + 1

print(mx, my)

print(len(bytes), sizeee)

bytess = bytearray(bytes)
file.write(bytess)

file.close()