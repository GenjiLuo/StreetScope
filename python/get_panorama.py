#!/usr/bin/python
from optparse import OptionParser
import StringIO
import Image
import eventlet

from third_party.adq_street_view import GetPanoramaTile

pool = eventlet.GreenPool(100)

def get_panorama_image(panoid, zoom):
    height = 208 * 2**zoom
    width = 2 * height

    ytiles = (height + 511) / 512
    xtiles = (width + 511) / 512

    img = Image.new('RGB', (width, height))

    def worker(x, y):
        imgdata = StringIO.StringIO(GetPanoramaTile(panoid, zoom, x, y))
        subimg = Image.open(imgdata)
        img.paste(subimg, (512 * x, 512 * y))

    for x in range(xtiles):
        for y in range(ytiles):
            pool.spawn(worker, x, y)
    pool.waitall()
    return img

def write_panorama_image(panoid, zoom, fn):
    img = get_panorama_image(panoid, zoom)
    img.save(fn)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-z', '--zoom', dest='zoom', default=3)
    parser.add_option('-f', '--outfile', dest='outfile', default='pano.jpg')

    opts, args = parser.parse_args()

    panoid = args[0]
    zoom = int(opts.zoom)
    fn = opts.outfile

    write_panorama_image(panoid, zoom, fn)
