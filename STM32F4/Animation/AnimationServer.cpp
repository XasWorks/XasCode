/*
 * AnimationServer.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#include <Animation/AnimationServer.h>
#include <Animation/AnimationElement.h>

#include <string.h>
#include <stdlib.h>

namespace Xasin {

AnimationServer::AnimationServer()
	: animations(),
	  needs_relink(false), needs_deletion(false),
	  synch_time(0) {

	osMutexAttr_t mutex_attr = {};
	mutex_attr.attr_bits = osMutexRecursive;

	animations_mutex = osMutexNew(&mutex_attr);
}

void AnimationServer::remove_pointer(AnimationElement *elem) {

	osMutexAcquire(animations_mutex, 0);

	for(auto i = animations.begin(); i < animations.end(); i++) {
		if(*i == elem)
			animations.erase(i);
	}

	osMutexRelease(animations_mutex);
}
void AnimationServer::insert_pointer(AnimationElement *elem) {
	force_relink();

	osMutexAcquire(animations_mutex, 0);

	for(auto i = animations.begin(); i<animations.end(); i++) {
		if(elem->draw_z < (*i)->draw_z) {
			animations.insert(i, elem);

			osMutexRelease(animations_mutex);
			return;
		}
	}

	animations.push_back(elem);

	osMutexRelease(animations_mutex);
}

void AnimationServer::force_relink() {
	needs_relink = true;
}

AnimationElement * AnimationServer::get_animation(animation_id_t id) {
	AnimationElement * out = nullptr;

	osMutexAcquire(animations_mutex, 0);

	for(auto anim : animations) {
		if(anim->ID.uniq_id == id.uniq_id) {
			if(anim->delete_after != 0 && anim->delete_after < synch_time)
				continue;
			out = anim;
			break;
		}
	}

	osMutexRelease(animations_mutex);

	return out;
}
void AnimationServer::delete_animation(animation_id_t animation) {
	osMutexAcquire(animations_mutex, 0);

	for(auto it = animations.begin(); it != animations.end(); it++) {
		if((*it)->ID.uniq_id == animation.uniq_id)
			(*it)->delete_after = 1;
	}

	osMutexRelease(animations_mutex);

	needs_deletion = true;
}
void AnimationServer::delete_animation_set(uint8_t set_no) {
	osMutexAcquire(animations_mutex, 0);

	for(auto it = animations.begin(); it != animations.end(); it++) {
		if((*it)->ID.set_id == set_no)
			(*it)->delete_after = 1;
	}

	osMutexRelease(animations_mutex);

	needs_deletion = true;
}

float * AnimationServer::get_float_ptr(animation_global_id_t id) {
	auto anim = get_animation(id.ID);

	if(anim == nullptr)
		return nullptr;

	return anim->get_flt(id.value);
}

void AnimationServer::tick(float delta_t) {
	synch_time += delta_t;

	osMutexAcquire(animations_mutex, 0);

	if(needs_deletion) {
		for(auto element = animations.begin(); element != animations.end();) {
			const float d_time = (*element)->delete_after;
			if((d_time != 0) && (d_time < synch_time)) {
				auto ptr = *element;
				element = animations.erase(element);

				delete ptr;

				needs_relink = true;
			}
			else
				element++;
		}
		needs_deletion = false;
	}

	if(needs_relink) {
		for(auto element : animations)
			element->relink();
	}
	needs_relink = false;

	for(auto element : animations) {
		if(element->delete_after < synch_time)
			needs_deletion = true;

		element->copy_tick(delta_t);
		element->tick(delta_t);
	}

	osMutexRelease(animations_mutex);
}

float AnimationServer::get_synch_time() {
	return synch_time;
}

animation_global_id_t AnimationServer::decode_value_tgt(const char *str) {
	animation_global_id_t out = {};

	// Forward to the designating S, returning if not found.
	str = strchr(str, 'S');
	if(str == nullptr)
		return {};
	if(*(++str) == '\0')
		return {};


	int decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.set_id = decoded_num;

	// Forward to 'M', returning if not found.
	while (*(str++) != 'M') {
		if(*str == '\0')
			return {};
	};

	decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.module_id = decoded_num;

	str = strchr(str, 'V');
	if(str == nullptr)
		return out;
	if(*(++str) == '\0')
		return out;

	out.value = strtol(str, nullptr, 16);

	return out;
}

// Input to this function is assumed to be:
// SxxMxxVxx .......;SxxMxxVxx 2123;2.1;4124;etc.
void AnimationServer::handle_set_command(const char *command) {
	animation_global_id_t target = {};

	while(command != nullptr && *command != '\0') {
		if(*command == 'S') {
			target = decode_value_tgt(command);
			command = strchr(command, ' ');
			if(command++ == nullptr)
				break;
		}
		else
			target.value++;

		auto v_ptr = get_animation(target.ID);

		if(v_ptr != nullptr)
			v_ptr->set_flt(target.value, command);

		command = strchr(command, ';');
		if(command++ == nullptr)
			break;
	}
}

void AnimationServer::handle_color_set_command(const char *command) {
	animation_global_id_t target = {};

	while(command != nullptr && *command != '\0') {
		if(*command == 'N')
			target.value++;
		else
			target = decode_value_tgt(command);

		auto v_ptr = get_animation(target.ID);

		if(v_ptr != nullptr)
			v_ptr->set_color(target.value, strchr(command, ' ')+1);

		command = strchr(command, ';');
		if(command == nullptr)
			break;

		command += 1;
	}
}

void AnimationServer::handle_delete_command(const char *command) {
	while(command != nullptr && *command != '\0') {
		if(strncmp(command, "SET ", 4) == 0) {
			if((command = strchr(command, ' ')) == nullptr)
				return;

			delete_animation_set(atoi(command));
		}
		else {
			delete_animation(decode_value_tgt(command).ID);
		}

		command = strchr(command, ';');
		if(command == nullptr)
			break;

		command += 1;
	}
}

void AnimationServer::handle_dtime_command(const char *command) {
	while(command != nullptr && *command != '\0') {
		auto v_ptr = get_animation(decode_value_tgt(command).ID);

		if(v_ptr != nullptr) {
			auto t_ptr = strchr(command, ' ')+1;
			if(*t_ptr == 'N')
				v_ptr->delete_after = 0;
			else
				v_ptr->delete_after = synch_time + strtof(t_ptr, nullptr);
		}

		command = strchr(command, ';');
		if(command == nullptr)
			break;

		command += 1;
	}
}

} /* namespace Xasin */
