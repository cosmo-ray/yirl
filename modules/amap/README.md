# a simple platformer/metroidvania module.

## Brief

it use json, and it's own ascii text format to define map.

each level must have a file using it's name, and a json file assiciate to it

example: `lvl0` will be a file containing an ascii representation of a level, and `lvl0.json`
will be use to make the corespondance from what's in `lvl0` and `lvl0.json`

## ASCII Format in-depth:

- `^`: represent a pike.
- `\``: represent a half horizontal wall.
- `#`: represent a big square wall.
- `@`: represent the default starting position of PC.
- `a-z`: are monsters, each map can have up to 24 kind of monsters per maps
- `0-9`: represent a kind of object
- `A-M`: represent a background obj.

Anything else is not yet used.

## json Format in-brief:

| name | type | desc |
|-|-|-|
| size | Array | size of map |
| objs | Array of Array | most be lower than 10, describe an object, objs[0], will describe the action to do when activate `0` on map |
| objs_condition | Array of Array or null | a condition callback, to know if onj need activation |
| background | string | map image or color background |
| background_auto_scale | bool | if true, force background size to a fix size |
| # | Object or string | by default block are plain colored square, this enable you to change that |
| ` | Object or string | same as `#` but for `\`` |
| monsters | object of object | describe montsters |
| next,next1,next2 | string | to be use in combinaison with callback of the same name |

## example
example of a simple lvl:

```
            f
          ###
 @   ````              r    0
###             ###############

```

and lvl.json:
```json
{
    "size": [35, 15],
    "#": {
	"path": "./img.png",
	"src_rect": [0, 0, 32, 32]
    },
    "'": "rgba: 127 127 127 100",
    "objs": [
	["door", "amap.next"]
    ],
    "background": "my-background.png",
    "monsters": {
	"v": {
	    "life": 5,
	    "action": ["flyer_movement_callback", "my arg"],
	    "dead": "amap.monster_dead",
	    "animation": "flyer",
	    "max_life": 2
	},
	"r": {
	    "life": 5,
	    "action": ["rat_movement_callback", "my arg"],
	    "dead": "amap.monster_dead",
	    "img": "static_rat_img",
	    "max_life": 2
	}
    },
    "next": "next_map"
}
```

