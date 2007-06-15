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
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.SystemUnavaliableException;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Block.action")
public class BlockUserActionBean extends WebmopsActionBean {
	
	private Person user;
	private Long userId;
	private PersonManager personDAO;
	
	public Person getUser() {
		return this.user;
	}
	
	public void setUser(Person user) {
		this.user = user;
	}
	
	public Long getUserId() {
		return this.userId;
	}
	
	public void setUserId(Long userId) {
		this.userId = userId;
	}
	
	@DefaultHandler
	public Resolution index() {
		return new RedirectResolution("/Default.action");
	}
	
	@HandlesEvent("block")
	public Resolution block() {
		
		personDAO = new PersonManager();
		
		user = personDAO.findPersonById(this.userId);
		
		if ( user != null ) {
			user.setActive(false);
			try {
				personDAO.saveOrUpdate(user);
			} catch (SystemUnavaliableException e) {
				e.printStackTrace();
			}
		}
		
		return getContext().getSourcePageResolution();
	}
}
