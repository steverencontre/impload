/*
	TrackView GPS viewer and high quality printer

	Copyright (c) 2010-11 Steve Rencontre	q.tv@rsn-tech.co.uk

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Chooser.h"
#include "ui_Chooser.h"

Chooser::Chooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Chooser)
{
    ui->setupUi(this);
}

Chooser::~Chooser()
{
    delete ui;
}
