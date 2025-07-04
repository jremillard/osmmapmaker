import sys
import os
import xml.etree.ElementTree as ET

COLORS = ['red','blue','green','orange','purple','cyan','magenta','brown','lime','darkblue','pink','gray']

# Parse OSM

def parse_osm(path):
    tree = ET.parse(path)
    root = tree.getroot()
    nodes = {}
    node_tags = {}
    for node in root.findall('node'):
        nid = node.attrib['id']
        lat = float(node.attrib['lat'])
        lon = float(node.attrib['lon'])
        nodes[nid] = (lon, lat)
        node_tags[nid] = {t.attrib['k']: t.attrib['v'] for t in node.findall('tag')}
    ways = []
    for way in root.findall('way'):
        ndrefs = [nd.attrib['ref'] for nd in way.findall('nd')]
        tags = {t.attrib['k']: t.attrib['v'] for t in way.findall('tag')}
        ways.append({'refs':ndrefs,'tags':tags})
    relations = []
    for rel in root.findall('relation'):
        members = []
        for m in rel.findall('member'):
            members.append({'type':m.attrib['type'], 'ref':m.attrib['ref'], 'role':m.attrib.get('role','')})
        tags = {t.attrib['k']: t.attrib['v'] for t in rel.findall('tag')}
        relations.append({'members':members,'tags':tags})
    return nodes, node_tags, ways, relations

def render_svg(nodes, node_tags, ways, relations, out_path):
    if not nodes:
        return
    lon_vals = [v[0] for v in nodes.values()]
    lat_vals = [v[1] for v in nodes.values()]
    minx = min(lon_vals); maxx = max(lon_vals)
    miny = min(lat_vals); maxy = max(lat_vals)
    width = 200; height = 200
    def sx(x): return (x-minx)/(maxx-minx or 1)*width
    def sy(y): return height - (y-miny)/(maxy-miny or 1)*height
    color_idx = 0
    elems = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}">']
    # Draw polygons and lines
    for way in ways:
        pts = [ (sx(nodes[r][0]), sy(nodes[r][1])) for r in way['refs'] if r in nodes ]
        if not pts:
            continue
        color = COLORS[color_idx % len(COLORS)]; color_idx +=1
        path_data = ' '.join(f"{x},{y}" for x,y in pts)
        if way['refs'][0]==way['refs'][-1] or way['tags'].get('area')=='yes':
            elems.append(f'<polygon points="{path_data}" fill="{color}" stroke="black" stroke-width="1" fill-opacity="0.5"/>')
        else:
            elems.append(f'<polyline points="{path_data}" fill="none" stroke="{color}" stroke-width="2"/>')
    # Draw relation boundaries as purple
    for rel in relations:
        outer = []
        for m in rel['members']:
            if m['type']=='way' and m['role']=='outer':
                # find way by id
                for w in ways:
                    if w['refs'] and w.get('id')==m['ref']:
                        pts = [ (sx(nodes[r][0]), sy(nodes[r][1])) for r in w['refs'] if r in nodes ]
                        outer = pts
                        break
        if outer:
            path_data = ' '.join(f"{x},{y}" for x,y in outer)
            color = COLORS[color_idx % len(COLORS)]; color_idx +=1
            elems.append(f'<polygon points="{path_data}" fill="{color}" stroke="black" stroke-width="1" fill-opacity="0.5"/>')
    # Draw points
    for nid,(lon,lat) in nodes.items():
        color = COLORS[color_idx % len(COLORS)]; color_idx +=1
        elems.append(f'<circle cx="{sx(lon)}" cy="{sy(lat)}" r="3" fill="{color}"/>')
    elems.append('</svg>')
    with open(out_path,'w') as f:
        f.write('\n'.join(elems))

def main():
    if len(sys.argv)!=3:
        print('Usage: osm_to_svg.py input.osm output.svg')
        return
    nodes,node_tags,ways,relations = parse_osm(sys.argv[1])
    render_svg(nodes,node_tags,ways,relations,sys.argv[2])

if __name__=='__main__':
    main()
