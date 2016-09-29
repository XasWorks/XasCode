
// Currently used supervariables are:
// $shown - A list of all tags that /WILL/ be shown,
// $not_shown - A list of all tags that will, on default, not be shown,
// $colortable - A table for the colors of different tags,
// $color_render_warning - A variable to make sure the "RENDERING DISABLED" warning is spewed only once
// $highlighted - A list of all tags that should be highlighted (ghosted),
// $opacity - a "pass down" value to ensure that all elements of a tree that should only be "ghosted" ARE only ghosted.

function contains(sArray, fString) = (len(sArray) > 0) ?
													max( [for(i = [0:len(sArray) -1]) sArray[i] == fString ? 1 : 0 ]) == 1
													: false;
function mContains(sArray, fArray) = ((len(sArray) > 0) && (len(fArray) > 0)) ?
													max( [for(i = [0:len(fArray) -1]) contains(sArray, fArray[i]) ? 1 : 0] ) == 1
													: false;


module color_appropriately(tagname) {
	if(!contains($colortable, tagname)) {
		children();
	}
	else {
		coloring = $colortable[search([tagname], $colortable, 1, 0)[0] + 1];

		color(coloring, $opacity)
		render(convexity = 5) children();
	}
}

module separate_tagging() {
	$shown = 0;
	$not_shown = 0;
	children();
}

module conditionalRender(rnder = true) {
	if(rnder) {
		render(convexity = 5) children();
	}
	else {
		children();
	}
}

// The tagging function, the core of the system.
// It only shows a module under following circumstances:
// -- It is listed in the $shown array
// -- The $show array is empty and it is not excluded (via $not_shown) AND it is a foreground object
module tag(tagname, foreground = true, rnder = true) {
	show_on = concat(tagname);
	color_name = show_on[0];

	color_appropriately(color_name) conditionalRender(rnder) {
		// If the "shown" array has any entries, ONLY the "shown" entries should be displayed.
		if(len($shown) > 0) {
			if(mContains($shown, show_on)) {
				separate_tagging() children();
			}
		}
		else if(foreground && !mContains($not_shown, show_on)) {
			separate_tagging() children();
		}
	}


	if(mContains($highlighted, show_on) && !mContains($shown, show_on)) {
		$opacity = 0.2;	// The "opacity" tag is shared down the CSG tree, also making any children opaque
		%color_appropriately(color_name)
			separate_tagging() children();
	}
}

module showOnly(tagname) {
	$shown = concat(tagname);
	children();
}

module showTag(tagname) {
	if(!(contains($shown, tagname))) {
		$shown = concat($shown, tagname);
		children();
	}
	else
		children();
}

module hideTag(tagname) {
	if(!(contains($not_shown, tagname) || contains($shown, tagname))) {
		$not_shown = concat($not_shown, tagname);
		children();
	}
	else
		children();
}

module colorTag(tagname, coloring) {
	$colortable = concat($colortable, [tagname, coloring]);

	if($color_render_warning == undef) {
		echo("WARNING - Coloration disables automatic preview rendering in tagged binary operators!");
		$color_render_warning = true;
		children();
	}
	else {
		children();
	}

}

module highlightTag(tagname) {
	$highlighted = concat($highlighted, tagname);

	children();
}

module taggedUnion(targets, tagname, foreground = true) {
	shouldRender = (len($colortable) == undef);

	union() {
		tag(tagname = tagname, foreground = foreground, rnder = shouldRender)
		union() {
			showTag(targets) children();
		}

		hideTag((foreground) ? targets : "EMPTY_TAGLIST_DONOTUSE") children();
	}
}

module taggedDifference(positives, negatives, tagname, foreground = true) {
	shouldRender = (len($colortable) == undef);

	union() {
		tag(tagname = tagname, foreground = foreground, rnder = shouldRender)
		difference() {
			showTag(positives) children();
			showTag(negatives) children();
		}

		hideTag((foreground) ? concat(positives, negatives) : "EMPTY_TAGLIST_DONOTUSE") children();
	}
}

module taggedIntersection(targets, tagname, foreground = true) {
	shouldRender = (len($colortable) == undef);

	union() {
		tag(tagname = tagname, foreground = foreground, rnder = shouldRender)
		intersection_for(i = targets) {
			showTag(i) children();
		}

		hideTag((foreground) ? targets : "EMPTY_TAGLIST_DONOTUSE") children();
	}
}

colorTag("negative", "red")
taggedDifference("positive", "negative", "neutral", true) {
	tag(["sphere1", "positive"]) sphere(r=10);
	tag("negative") cube(15, true);
}
