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
   
package ru.rpunet.webmops.model;

import java.io.Serializable;
import java.util.List;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.JoinColumn;
import javax.persistence.OneToMany;
import javax.persistence.Table;
import javax.persistence.Id;

import org.hibernate.validator.NotNull;

/**
 * Distribution entity
 * distribution has many mirros
 * @author andrew
 *
 */
@Entity
@Table(name="distributions")
public class Distribution implements Serializable {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 9085662363289959746L;

	@Id
	@GeneratedValue(strategy=GenerationType.IDENTITY)
	private Long Id;
	
	@NotNull
	private String name;
	
	@NotNull
	private String description;
	
	@OneToMany
	@JoinColumn(name="distribution_id")
	private List<Mirror> mirrors;

	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	public List<Mirror> getMirrors() {
		return mirrors;
	}

	public void setMirrors(List<Mirror> mirros) {
		this.mirrors = mirros;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public Long getId() {
		return Id;
	}

}
