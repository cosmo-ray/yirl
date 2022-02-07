An examples of shooter module for yirl

here a configuration examples
```json
{
	"<type>" : "asteroide-shooter", // <-- name still need to be change
	 "type": "skull-breaker", // If Skull breaker is play in "skull-breaker" mode, otherwise it's asteroid shooter
   "lvlup": [ // what happen on level up, a skill is randmly choose, displaying "img" and "action" is call
	    {
		    "img": "go_go_muscle.png",
		    "action": "asteroide-shooter.increase_atk_speed"
	    },
	    {
     		"img": "shojo spirit.png",
		    "action": "asteroide-shooter.attack_nb_1"
	    }
	   ]
}
```
