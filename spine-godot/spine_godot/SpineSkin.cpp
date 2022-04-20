/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated January 1, 2020. Replaces all prior versions.
 *
 * Copyright (c) 2013-2020, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "SpineSkin.h"
#include "SpineBoneData.h"
#include "SpineConstraintData.h"
#include "SpineCommon.h"

void SpineSkin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "name"), &SpineSkin::init);
	ClassDB::bind_method(D_METHOD("set_attachment", "slot_index", "name", "attachment"), &SpineSkin::set_attachment);
	ClassDB::bind_method(D_METHOD("get_attachment", "slot_index", "name"), &SpineSkin::get_attachment);
	ClassDB::bind_method(D_METHOD("remove_attachment", "slot_index", "name"), &SpineSkin::remove_attachment);
	ClassDB::bind_method(D_METHOD("find_names_for_slot", "slot_index"), &SpineSkin::find_names_for_slot);
	ClassDB::bind_method(D_METHOD("find_attachments_for_slot", "slot_index"), &SpineSkin::find_attachments_for_slot);
	ClassDB::bind_method(D_METHOD("get_name"), &SpineSkin::get_name);
	ClassDB::bind_method(D_METHOD("add_skin", "other"), &SpineSkin::add_skin);
	ClassDB::bind_method(D_METHOD("copy_skin", "other"), &SpineSkin::copy_skin);
	ClassDB::bind_method(D_METHOD("get_attachments"), &SpineSkin::get_attachments);
	ClassDB::bind_method(D_METHOD("get_bones"), &SpineSkin::get_bones);
	ClassDB::bind_method(D_METHOD("get_constraints"), &SpineSkin::get_constraints);
}

SpineSkin::SpineSkin() : skin(nullptr), owns_skin(false) {
}

SpineSkin::~SpineSkin() {
	if (owns_skin) delete skin;
}

Ref<SpineSkin> SpineSkin::init(const String &name) {
	if (skin) {
		ERR_PRINT("Can not initialize an already initialized skin.");
		return this;
	}
	owns_skin = true;
	skin = new spine::Skin(SPINE_STRING(name));
	return this;
}

void SpineSkin::set_attachment(uint64_t slot_index, const String &name, Ref<SpineAttachment> attachment) {
	SPINE_CHECK(skin,)
	skin->setAttachment(slot_index, SPINE_STRING(name), attachment.is_valid() ? attachment->get_spine_object() : nullptr);
}

Ref<SpineAttachment> SpineSkin::get_attachment(uint64_t slot_index, const String &name) {
	SPINE_CHECK(skin, nullptr)
	auto attachment = skin->getAttachment(slot_index, SPINE_STRING(name));
	if (attachment) return nullptr;
	Ref<SpineAttachment> attachment_ref(memnew(SpineAttachment));
	attachment_ref->set_spine_object(attachment);
	return attachment_ref;
}

void SpineSkin::remove_attachment(uint64_t slot_index, const String &name) {
	SPINE_CHECK(skin,)
	skin->removeAttachment(slot_index, SPINE_STRING(name));
}

Array SpineSkin::find_names_for_slot(uint64_t slot_index) {
	Array result;
	SPINE_CHECK(skin, result)
	spine::Vector<spine::String> names;
	skin->findNamesForSlot(slot_index, names);
	result.resize((int)names.size());
	for (int i = 0; i < names.size(); ++i) {
		result[i] = names[i].buffer();
	}
	return result;
}

Array SpineSkin::find_attachments_for_slot(uint64_t slot_index) {
	Array result;
	SPINE_CHECK(skin, result)
	spine::Vector<spine::Attachment *> attachments;
	skin->findAttachmentsForSlot(slot_index, attachments);
	result.resize((int)attachments.size());
	for (int i = 0; i < attachments.size(); ++i) {
		if (!attachments[i]) {
			result[i] = Ref<SpineAttachment>(nullptr);
		} else {
			Ref<SpineAttachment> attachment_ref(memnew(SpineAttachment));
			attachment_ref->set_spine_object(attachments[i]);
			result[i] = attachment_ref;
		}
	}
	return result;
}

String SpineSkin::get_name() {
	SPINE_CHECK(skin, "")
	return skin->getName().buffer();
}

void SpineSkin::add_skin(Ref<SpineSkin> other) {
	SPINE_CHECK(skin,)
	if (!other.is_valid() || !other->get_spine_object()) {
		ERR_PRINT("other is not a valid SpineSkin.");
		return;
	}
	skin->addSkin(other->get_spine_object());
}

void SpineSkin::copy_skin(Ref<SpineSkin> other) {
	SPINE_CHECK(skin,)
	if (!other.is_valid() || !other->get_spine_object()) {
		ERR_PRINT("other is not a valid SpineSkin.");
		return;
	}
	skin->copySkin(other->get_spine_object());
}

Array SpineSkin::get_attachments() {
	Array result;
	SPINE_CHECK(skin, result)
	auto entries = skin->getAttachments();
	while(entries.hasNext()) {
		spine::Skin::AttachmentMap::Entry &entry = entries.next();
		Ref<SpineSkinEntry> entry_ref = memnew(SpineSkinEntry);
		Ref<SpineAttachment> attachment_ref = nullptr;
		if (entry._attachment) {
			attachment_ref = Ref<SpineAttachment>(memnew(SpineAttachment));
			attachment_ref->set_spine_object(entry._attachment);
		}
		entry_ref->init(entry._slotIndex, entry._name.buffer(), attachment_ref);
		result.push_back(entry_ref);
	}
	return result;
}

Array SpineSkin::get_bones() {
	Array result;
	SPINE_CHECK(skin, result)
	auto bones = skin->getBones();
	result.resize((int)bones.size());
	for (int i = 0; i < bones.size(); ++i) {
		Ref<SpineBoneData> bone_ref(memnew(SpineBoneData));
		bone_ref->set_spine_object(bones[i]);
		result[i] = bone_ref;
	}
	return result;
}

Array SpineSkin::get_constraints() {
	Array result;
	SPINE_CHECK(skin, result)
	auto constraints = skin->getConstraints();
	result.resize((int)constraints.size());
	for (int i = 0; i < constraints.size(); ++i) {
		Ref<SpineConstraintData> constraint_ref(memnew(SpineConstraintData));
		constraint_ref->set_spine_object(constraints[i]);
		result[i] = constraint_ref;
	}
	return result;
}

void SpineSkinEntry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_slot_index"), &SpineSkinEntry::get_slot_index);
	ClassDB::bind_method(D_METHOD("get_name"), &SpineSkinEntry::get_name);
	ClassDB::bind_method(D_METHOD("get_attachment"), &SpineSkinEntry::get_attachment);
}

SpineSkinEntry::SpineSkinEntry() : slot_index(0) {
}

int SpineSkinEntry::get_slot_index() {
	return slot_index;
}

const String &SpineSkinEntry::get_name() {
	return name;
}

Ref<SpineAttachment> SpineSkinEntry::get_attachment() {
	return attachment;
}
