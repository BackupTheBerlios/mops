/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package ru.rpunet.webmops.web;

import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Profile.action")
public class UserProfileActionBean extends WebmopsActionBean {

	private Long userId;
	
	private Person user;

	public Person getUser() {
		return user;
	}

	public void setUser(Person user) {
		this.user = user;
	}

	public Long getUserId() {
		return userId;
	}

	public void setUserId(Long userId) {
		this.userId = userId;
	}
	
	@DefaultHandler
	public Resolution index() {
		PersonManager personDAO = new PersonManager();
		
		Long id;
		
		if ( this.userId == null ) {
			id = getContext().getUser().getId();
		} else {
			id = this.userId;
		}
		
		user = personDAO.findPersonById(id);
		
		return new ForwardResolution("/UserProfile.jsp");
	}
	
	@HandlesEvent("showPackages")
	public Resolution showPackages() {
		return new ForwardResolution("/PersonalPackagesList.jsp");
	}
	
}
